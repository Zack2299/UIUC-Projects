
import time
import math
import numpy as np
import matplotlib.pyplot as plt
import cv2
import rospy

from line_fit import line_fit, tune_fit, bird_fit, final_viz
from Line import Line
from sensor_msgs.msg import Image
from std_msgs.msg import Header
from cv_bridge import CvBridge, CvBridgeError
from std_msgs.msg import Float32
from skimage import morphology

import random

class lanenet_detector():
    def __init__(self):

        self.bridge = CvBridge()
        # NOTE
        # Uncomment this line for lane detection of GEM car in Gazebo
        self.sub_image = rospy.Subscriber('/gem/front_single_camera/front_single_camera/image_raw', Image, self.img_callback, queue_size=1)
        # Uncomment this line for lane detection of videos in rosbag
        # self.sub_image = rospy.Subscriber('camera/image_raw', Image, self.img_callback, queue_size=1)
        #830
        # self.sub_image = rospy.Subscriber('/zed2/zed_node/rgb/image_rect_color', Image, self.img_callback, queue_size=1)

        self.pub_image = rospy.Publisher("lane_detection/annotate", Image, queue_size=1)
        self.pub_bird = rospy.Publisher("lane_detection/birdseye", Image, queue_size=1)
        self.left_line = Line(n=5)
        self.right_line = Line(n=5)
        self.detected = False
        self.hist = True


    def img_callback(self, data):

        try:
            # Convert a ROS image message into an OpenCV image
            cv_image = self.bridge.imgmsg_to_cv2(data, "bgr8")
        except CvBridgeError as e:
            print(e)

        raw_img = cv_image.copy()
        mask_image, bird_image = self.detection(raw_img)

        if mask_image is not None and bird_image is not None:
            # Convert an OpenCV image into a ROS image message
            out_img_msg = self.bridge.cv2_to_imgmsg(mask_image, 'bgr8')
            out_bird_msg = self.bridge.cv2_to_imgmsg(bird_image, 'bgr8')

            # Publish image message in ROS
            self.pub_image.publish(out_img_msg)
            self.pub_bird.publish(out_bird_msg)


    # def gradient_thresh(self, img, thresh_min=25, thresh_max=100):
    #     """
    #     Apply sobel edge detection on input image in x, y direction
    #     """
    #     #1. Convert the image to gray scale
    #     #2. Gaussian blur the image
    #     #3. Use cv2.Sobel() to find derievatives for both X and Y Axis
    #     #4. Use cv2.addWeighted() to combine the results
    #     #5. Convert each pixel to uint8, then apply threshold to get binary image

    #     ## TODO

    #     img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    #     img_blurred = cv2.GaussianBlur(img_gray, ksize = 1)

    #     dx = cv2.Sobel(img_blurred, dx, -1, 1, 0, 1)
    #     dy = cv2.Sobel(img_blurred, dy, -1, 0, 1, 1)

    #     combined = cv2.addWeighted(dx, .5, dy, .5, 0)

    #     combined_8 = cv2.convertScaleAbs(combined, alpha=(255.0/65535.0))

    #     # binary_output = cv2.threshhold(thresh_min, thresh_max)
    #     binary_output = cv2.threshold(combined_8, thresh_min, thresh_max, cv2.THRESH_BINARY)


    #     ###

    #     return binary_output

    # def gradient_thresh(self, img, thresh_min=25, thresh_max=100):
    #     """
    #     Apply Sobel edge detection on input image in x, y direction
    #     """
    #     try:
    #         # Check the data type of img
    #         print("Image data type:", type(img))
           
    #         # Ensure img is not None
    #         while img is None:
    #             print("hi")
           
    #         # Convert the image to grayscale
    #         img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
           
    #         # Gaussian blur the image
    #         img_blurred = cv2.GaussianBlur(img_gray, (5, 5), 0)

    #         dx = cv2.Sobel(img_blurred, cv2.CV_64F, 1, 0, ksize=3)
    #         dy = cv2.Sobel(img_blurred, cv2.CV_64F, 0, 1, ksize=3)

    #         combined = cv2.addWeighted(dx, 0.5, dy, 0.5, 0)
           
    #         combined_8 = cv2.convertScaleAbs(combined)

    #         _, binary_output = cv2.threshold(combined_8, thresh_min, thresh_max, cv2.THRESH_BINARY)

    #         return binary_output
    #     except Exception as e:
    #         print("Error in gradient_thresh:", e)


    def gradient_thresh(self, img, thresh_min=25, thresh_max=100):
        """
    #     Apply sobel edge detection on input image in x, y direction
    #     """
    #     #1. Convert the image to gray scale
    #     #2. Gaussian blur the image
    #     #3. Use cv2.Sobel() to find derievatives for both X and Y Axis
    #     #4. Use cv2.addWeighted() to combine the results
    #     #5. Convert each pixel to uint8, then apply threshold to get binary image

    #     ## TODO

        # Convert the image to grayscale
        img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
       
        # Gaussian blur the image
        img_blurred = cv2.GaussianBlur(img_gray, (3, 3), 0)

        # dx = cv2.Sobel(img_blurred, cv2.CV_64F, 1, 0, ksize=3)
        # dy = cv2.Sobel(img_blurred, cv2.CV_64F, 0, 1, ksize=3)
        dx = cv2.Sobel(img_blurred, cv2.CV_64F, 1, 0, ksize=3)
        dy = cv2.Sobel(img_blurred, cv2.CV_64F, 0, 1, ksize=3)

        if (img.shape[1] == 1280):
            combined = cv2.addWeighted(dx, 0.90, dy, 0.10, 0)
        else:
            combined = cv2.addWeighted(dx, 0.82, dy, 0.18, 0)
       
        combined_8 = cv2.convertScaleAbs(combined)
        # combined_8 = (combined * 255).round().astype(np.uint8)
        # combined_8 = np.uint8(combined)

        # filter_combined = thresh_min < combined and combined < thresh_max

        # combined[filter_combined] = 1
 
        # binary_output = combined

        _, binary_output = cv2.threshold(combined_8, thresh_min, 255, cv2.THRESH_BINARY)

        # binary_output[binary_output == 255] = 1

        binary_output = np.divide(binary_output, 255, out=binary_output, casting='unsafe')
        cv2.imwrite("gradient_binary_output.png", 255* binary_output)
        # cv2.waitKey(0)
        # print(binary_output)

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
        s_channel = img_hls[:, :, 2]
        # _, h_output = cv2.threshold(h_channel, threshold, 255, cv2.THRESH_BINARY)
        # _, binary_output = cv2.threshold(s_channel, threshold, thresh[1], cv2.THRESH_BINARY)
        # print(binary_output.shape)
        s_binary = np.zeros_like(s_channel)
        s_binary[(s_channel > 100) & (s_channel <= 255)] = 1

        h_binary = np.zeros_like(h_channel)
        h_binary[(h_channel > 0) & (h_channel <= 45)] = 1

        binary_output = np.zeros_like(s_binary)
        binary_output[(s_binary == 1) & (h_binary == 1)] = 1
        # binary_output *= 255
        

        # img_hls[:,:,0] = binary_output
        # thresh_h = 1
        # threshold_h = 20
        # _, binary_output = cv2.threshold(h_channel, threshold_h, thresh_h, cv2.THRESH_BINARY)
        # # img_hls[:,:,2] = binary_output

        # binary_output = np.divide(binary_output, 255, out=binary_output, casting='unsafe')

        # print(binary_output)

        # combined = cv2.addWeighted(h_output, 0.5, s_output, 0.5, 0)
       
        # binary_output = cv2.convertScaleAbs(combined)


        ####

        # cv2.imwrite("h_output.png", h_output)
        cv2.imwrite("binary_output.png", 255* binary_output)
        # cv2.waitKey(0)

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
        cv2.imwrite("SobelOutput.png", SobelOutput)
        # ColorOutput = self.color_thresh(img)
        ColorOutput = self.color_thresh(img, thresh=(100,255))
        # print(ColorOutput)
        cv2.imwrite("ColorOutput.png", ColorOutput)

        ####

        binaryImage = np.zeros_like(SobelOutput)
        binaryImage[(ColorOutput==1)|(SobelOutput==1)] = 1
        # Remove noise from binary image
        binaryImage = morphology.remove_small_objects(binaryImage.astype('bool'),min_size=50,connectivity=2)

        # binaryImage = np.zeros_like(SobelOutput)
        # binaryImage = np.zeros_like(ColorOutput)
        # for i in range(ColorOutput.shape[0]):
        #     for j in range(ColorOutput.shape[1]):
        #         binaryImage[i][j] = 255*((ColorOutput[i][j]==255)|(SobelOutput[i][j]==255))
               
        # Remove noise from binary image
        # binaryImage = morphology.remove_small_objects(binaryImage.astype('bool'),min_size=50,connectivity=2)


        cv2.imwrite("combined.png", 255*np.float32(binaryImage))
        return binaryImage


    def perspective_transform(self, img, verbose=False):
        """
        Get bird's eye view from input image
        """
        #1. Visually determine 4 source points and 4 destination points
        #2. Get M, the transform matrix, and Minv, the inverse using cv2.getPerspectiveTransform()
        #3. Generate warped image in bird view using cv2.warpPerspective()

        ## TODO
        # width = 640
        # height = 480
        width = img.shape[1]
        height = img.shape[0]

        # tl = (235, 255)
        # tr = (438, 257)
        # bl = (0, 320)
        # br = (639, 294)

        # tl = (285, 255)
        # tr = (410, 257)
        # bl = (50, 320)
        # br = (590, 294)

        # #SIMULATION:
        if (width == 640):
            tl = (241, 259) #was 289
            tr = (377, 259) #was 259
            bl = (21, 400)
            br = (618, 400) #was 325
        elif(width == 1280):
            tl = (int(0.403*width), int(0.527*height))
            tr = (int(0.545*width), int(0.527*height))
            bl = (int(0.122*width), int(0.927*height))
            br = (int(0.752*width), int(0.927*height))
        else:
            tl = (int(.446*width), int(.55*height))
            tr = (int(0.56*width), int(.55*height))
            bl = (int(0.244*width), int(0.89*height))
            br = (int(0.653*width), int(0.89*height))


        src = np.float32([list(tl), list(tr), list(bl), list(br)])
        dest = np.float32([[0,0], [width, 0], [0, height], [width, height]])
        M = cv2.getPerspectiveTransform(src, dest)
        Minv = cv2.getPerspectiveTransform(dest, src)
        warped_img = cv2.warpPerspective(np.uint8(img), M, (width, height))
        ####
        return warped_img, M, Minv


    def detection(self, img):

        binary_img = self.combinedBinaryImage(img)
        img_birdeye, M, Minv = self.perspective_transform(binary_img)

        if not self.hist:
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
            if not self.detected:
                ret = line_fit(img_birdeye)

                if ret is not None:
                    left_fit = ret['left_fit']
                    right_fit = ret['right_fit']
                    nonzerox = ret['nonzerox']
                    nonzeroy = ret['nonzeroy']
                    left_lane_inds = ret['left_lane_inds']
                    right_lane_inds = ret['right_lane_inds']

                    left_fit = self.left_line.add_fit(left_fit)
                    right_fit = self.right_line.add_fit(right_fit)

                    self.detected = True

            else:
                left_fit = self.left_line.get_fit()
                right_fit = self.right_line.get_fit()
                ret = tune_fit(img_birdeye, left_fit, right_fit)

                if ret is not None:
                    left_fit = ret['left_fit']
                    right_fit = ret['right_fit']
                    nonzerox = ret['nonzerox']
                    nonzeroy = ret['nonzeroy']
                    left_lane_inds = ret['left_lane_inds']
                    right_lane_inds = ret['right_lane_inds']

                    left_fit = self.left_line.add_fit(left_fit)
                    right_fit = self.right_line.add_fit(right_fit)

                else:
                    self.detected = False

            # Annotate original image
            bird_fit_img = None
            combine_fit_img = None
            if ret is not None:
                bird_fit_img = bird_fit(img_birdeye, ret, save_file=None)
                combine_fit_img = final_viz(img, left_fit, right_fit, Minv)
            else:
                print("Unable to detect lanes")

            return combine_fit_img, bird_fit_img


if __name__ == '__main__':
    # init args
    rospy.init_node('lanenet_node', anonymous=True)
    ld = lanenet_detector()
    path = "test.png"
    im = cv2.imread(path)
    bird_eye, M, Minc = ld.perspective_transform(im)
    #cv2.imshow("BE", bird_eye)
    cv2.imwrite("BE.png", bird_eye)
    # Wait for the user to press a key
    cv2.waitKey(0)
   
    # Close all windows
    cv2.destroyAllWindows()
    while not rospy.core.is_shutdown():
        rospy.rostime.wallsleep(0.5)