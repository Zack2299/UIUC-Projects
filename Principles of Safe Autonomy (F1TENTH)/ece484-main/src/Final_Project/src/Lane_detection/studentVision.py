import time
import math
import numpy as np
import matplotlib.pyplot as plt
import cv2
import rospy

import sys

from ZACcontroller import ZACcontroller
from pure_pursuit import PP

from line_fit import line_fit, tune_fit, bird_fit, final_viz, get_ref_points
from Line import Line
from sensor_msgs.msg import Image, LaserScan
from std_msgs.msg import Header
from cv_bridge import CvBridge, CvBridgeError
from std_msgs.msg import String, Bool, Float32, Float64, Float64MultiArray
from ackermann_msgs.msg import AckermannDriveStamped as ZackermannDriveStamped
from skimage import morphology

import random

class lanenet_detector():

    def __init__(self):
        self.pp = PP()
        self.bridge = CvBridge()
        # NOTE

        # Uncomment this line for lane detection of GEM car in Gazebo
        # self.sub_image = rospy.Subscriber('/gem/front_single_camera/front_single_camera/image_raw', Image, self.img_callback, queue_size=1)
        # Uncomment this line for lane detection of videos in rosbag
        self.sub_image = rospy.Subscriber('D435I/color/image_raw', Image, self.img_callback, queue_size=1)
        self.lidar_sub = rospy.Subscriber('scan', LaserScan, self.scan_callback, queue_size=10)

        #830
        # self.sub_image = rospy.Subscriber('/zed2/zed_node/rgb/image_rect_color', Image, self.img_callback, queue_size=1)

        self.pub_image = rospy.Publisher("lane_detection/annotate", Image, queue_size=1)
        self.pub_bird = rospy.Publisher("lane_detection/birdseye", Image, queue_size=1)
        self.pub_lane = rospy.Publisher("lane_detection/top_of_lane", Image, queue_size=1)
        self.controlPub = rospy.Publisher("/vesc/low_level/ackermann_cmd_mux/input/navigation", ZackermannDriveStamped, queue_size=1)

        self.drive_msg = ZackermannDriveStamped()
        self.drive_msg.header.frame_id = "f1tenth_control"
        self.drive_msg.drive.speed     = 0.6 # m/s, reference speed
        self.unsafe_dist = 0.8
        self.left_line = Line(n=5)
        self.right_line = Line(n=5)
        self.object_detected = False
        self.detected = False
        self.hist = True

    def scan_callback(self, msg):
        '''
        range length is 1081, range[0] is at angle min and range[1080] is at angle max, 
        so range[540] is in the middle, being angle 0, in front of car.
        each index is angle_increment apart, so we can limit front of car to +/- 1 degree of 0

        '''
        middle_idx = 540
        car_front_angle = 0.01745 # in radians
        angle_inc = 0.004363
        idx_from_middle = int(car_front_angle / angle_inc)
        min_idx = middle_idx - idx_from_middle
        max_idx = middle_idx + idx_from_middle

        lidar_dist = msg.ranges[min_idx-1:max_idx+1]
        # print(lidar_dist)
        if min(lidar_dist) < self.unsafe_dist:
            self.object_detected = True
        else:
            self.object_detected = False
        # print(self.object_detected)


    def img_callback(self, data):
        try:
            # Convert a ROS image message into an OpenCV image
            cv_image = self.bridge.imgmsg_to_cv2(data, "bgr8")

        except CvBridgeError as e:
            print(e)

        raw_img = cv_image.copy()
        mask_image, bird_image, lane_img = self.detection(raw_img)

        if mask_image is not None and bird_image is not None:
            # Convert an OpenCV image into a ROS image message
            out_img_msg = self.bridge.cv2_to_imgmsg(mask_image, 'bgr8')
            out_bird_msg = self.bridge.cv2_to_imgmsg(bird_image, 'bgr8')
            out_lane_msg = self.bridge.cv2_to_imgmsg(lane_img, 'bgr8')

            # Publish image message in ROS
            self.pub_image.publish(out_img_msg)
            self.pub_bird.publish(out_bird_msg)
            self.pub_lane.publish(out_lane_msg)

 
    def gradient_thresh(self, img, thresh_min=25, thresh_max=100):
        """

    #     Apply sobel edge detection on input image in x, y direction

    #     """
        #1. Convert the image to gray scale
        #2. Gaussian blur the image
        #3. Use cv2.Sobel() to find derievatives for both X and Y Axis
        #4. Use cv2.addWeighted() to combine the results
        #5. Convert each pixel to uint8, then apply threshold to get binary image

        ## TODO

        # Convert the image to grayscale
        img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        # Gaussian blur the image
        img_blurred = cv2.GaussianBlur(img_gray, (3, 3), 0)

        dx = cv2.Sobel(img_blurred, cv2.CV_64F, 1, 0, ksize=3)

        dy = cv2.Sobel(img_blurred, cv2.CV_64F, 0, 1, ksize=3)

        if (img.shape[1] == 1280):
            combined = cv2.addWeighted(dx, 0.90, dy, 0.10, 0)

        else:
            combined = cv2.addWeighted(dx, 0.82, dy, 0.18, 0)

        combined_8 = cv2.convertScaleAbs(combined)

        _, binary_output = cv2.threshold(combined_8, thresh_min, 255, cv2.THRESH_BINARY)

        binary_output = np.divide(binary_output, 255, out=binary_output, casting='unsafe')

        return binary_output



    def color_thresh(self, img, thresh=(100, 255)):

        """

        Convert RGB to HSL and threshold to binary image using S channel

        """

        #1. Convert the image from RGB to HSL
        #2. Apply threshold on S channel to get binary image
        #Hint: threshold on H to remove green grass

        ## TODO

        threshold = 155

        # image = np.array(img)
        img_hls = cv2.cvtColor(img, cv2.COLOR_BGR2HLS_FULL)
        # print(thresh[0])
        h_channel = img_hls[:, :, 0]
        # l_channel = img_hls[:, :, 0]
        s_channel = img_hls[:, :, 2]

        """
        45 was meh

        60 was better

        75 acceptable (lots of noise at dim to bright)

        80 acceptable (same issue with 75 but bright to dim sees directly in front only)

        """
        s_binary = np.zeros_like(s_channel)
        s_binary[(s_channel > 50) & (s_channel <= 255)] = 1        # used to be: 100 ~ 255      45 was meh, 60 was better, 75 acceptable (lots of noise at dim to bright)

        h_binary = np.zeros_like(h_channel)
        h_binary[(h_channel > 20) & (h_channel <= 60)] = 1           # used to be: 0 ~ 45

        binary_output = np.zeros_like(s_binary)
        binary_output[(s_binary == 1) & (h_binary == 1)] = 1

        return binary_output

 

    def combinedBinaryImage(self, img):
        """
        Get combined binary image from color filter and sobel filter

        """

        #1. Apply sobel filter and color filter on input image
        #2. Combine the outputs
        ## Here you can use as many methods as you want.

        ## TODO
        SobelOutput = self.gradient_thresh(img, thresh_min=25, thresh_max=100)

        ColorOutput = self.color_thresh(img, thresh=(100,255))

        ####

        binaryImage = np.zeros_like(SobelOutput)

        cutOff = img.shape[0] * .69

        binaryImage[(ColorOutput==1)|(SobelOutput==1)] = 1

        # Remove noise from binary image

        binaryImage = morphology.remove_small_objects(binaryImage.astype('bool'),min_size=50,connectivity=2)

        for i in range(int(cutOff)):

            binaryImage[i] = np.zeros_like(binaryImage[i])

        return ColorOutput

 

    def perspective_transform(self, img, verbose=False):

        """
        Get bird's eye view from input image
        """

        #1. Visually determine 4 source points and 4 destination points
        #2. Get M, the transform matrix, and Minv, the inverse using cv2.getPerspectiveTransform()
        #3. Generate warped image in bird view using cv2.warpPerspective()

        ## TODO

        width = img.shape[1]

        height = img.shape[0]

 
        #Final Project Second Points
        tl = (0, .69 * height)
        tr = (width, .69 * height)
        bl = (0, height)
        br = (width, height)

        src = np.float32([list(tl), list(tr), list(bl), list(br)])
        dest = np.float32([[0,0], [width, 0], [0, height], [width, height]])

        M = cv2.getPerspectiveTransform(src, dest)
        Minv = cv2.getPerspectiveTransform(dest, src)
        warped_img = cv2.warpPerspective(np.uint8(img), M, (width, height))

        ####

        return warped_img, M, Minv


    def detection(self, img):

        # FINE TUNE THESE
        img_width = 640/2
        img_height = 480
        margin = 80

        x_coordsTop = np.array([])
        x_coordsBottom = np.array([])

        # ---------------

        binary_img = self.combinedBinaryImage(img)
        img_birdeye, M, Minv = self.perspective_transform(binary_img)

        if not self.hist:                                   # never comes in here because flag is always True

            # Fit lane without previous result

            ret = line_fit(img_birdeye)

            left_fit = ret['left_fit']

            right_fit = ret['right_fit']

            nonzerox = ret['nonzerox']

            nonzeroy = ret['nonzeroy']

            left_lane_inds = ret['left_lane_inds']

            right_lane_inds = ret['right_lane_inds']


        else:

            # Fit lane with previous result

            if not self.detected:                           # LINEFIT: comes here once in the beginning and once after failing to detect a lane

                ret = line_fit(img_birdeye)

                if ret is not None:                         # LINEFIT: comes here once; if its NONE then prints CANNOT FIND LANES
                    left_fit = ret['left_fit']
                    right_fit = ret['right_fit']
                    nonzerox = ret['nonzerox']
                    nonzeroy = ret['nonzeroy']
                    left_lane_inds = ret['left_lane_inds']
                    right_lane_inds = ret['right_lane_inds']
 
                    left_fit = self.left_line.add_fit(left_fit)
                    right_fit = self.right_line.add_fit(right_fit)

                    self.detected = True


            else:                                           # TUNEFIT: comes here after successful line_fit                              

                left_fit = self.left_line.get_fit()
                right_fit = self.right_line.get_fit()
                ret = tune_fit(img_birdeye, left_fit, right_fit)

                if ret is not None:                         # TUNEFIT: if not NOTHING
                    left_fit = ret['left_fit']
                    right_fit = ret['right_fit']
                    nonzerox = ret['nonzerox']
                    nonzeroy = ret['nonzeroy']
                    left_lane_inds = ret['left_lane_inds']
                    right_lane_inds = ret['right_lane_inds']

                    left_fit = self.left_line.add_fit(left_fit)
                    right_fit = self.right_line.add_fit(right_fit)

                    averageXtop, averageXBottom, middleX = get_ref_points(ret['first_Y_pixels'], ret['last_Y_pixels'], ret['xpixels'], ret['middle_Y_pixels'])
                    righty = nonzeroy[right_lane_inds]

                    middleTuple = (int(middleX), righty[int(len(righty)/2)])
                    pixelPosTop = [averageXtop, ret['firsty']]
                    pixelPosbottom = [averageXBottom, ret['lasty']]

                    cur_state = (640/2, 0)

                    target_state = (middleX, 0)

                    triangle_tuple = (pixelPosTop[0]-img_width, pixelPosTop[1] - img_height)
 

                    theta = np.arctan(triangle_tuple[0]/triangle_tuple[1])

                    L = np.sqrt(triangle_tuple[0] ** 2 + triangle_tuple[1] ** 2)

                    #THIS IS PID
                    # self.zac_controller.compute_sig(target_state, cur_state, theta, L)
                    # stearing_angle = self.zac_controller.ZAC_signal[0]
                    # print("Radians ZAC: ", self.zac_controller.ZAC_signal

                    #THIS IS PP
                    stearing_angle = self.pp.start_pp(pixelPosTop, False)
                    # if pixelPosTop[1] >= 400:
                    #     self.drive_msg.drive.speed = 0
                    # else:
                    #     self .drive_msg.drive.speed = .6

                    # print(np.degrees(stearing_angle))
                    speed = 0
                    if(self.object_detected):
                        speed = 0
                        self.drive_msg.drive.speed = 0
                    else:
                        speed = 0.6
                        self.drive_msg.drive.speed = 0.6
                    print(speed, self.object_detected)
                    self.drive_msg.drive.steering_angle = stearing_angle
                    self.controlPub.publish(self.drive_msg)

                else:

                    self.detected = False

 

            # Annotate original image
            bird_fit_img = None
            combine_fit_img = None
            lane_fit_img = None
            if ret is not None:
                lane_fit_img, bird_fit_img = bird_fit(img_birdeye, ret, save_file=None)
                combine_fit_img = final_viz(img, left_fit, right_fit, Minv)

            else:
                print("Unable to detect lanes")

 
            return combine_fit_img, bird_fit_img, lane_fit_img

 

if __name__ == '__main__':

    # init args
    rospy.init_node('lanenet_node', anonymous=True)
    ld = lanenet_detector()

    while not rospy.core.is_shutdown():

        rospy.rostime.wallsleep(0.5)

 