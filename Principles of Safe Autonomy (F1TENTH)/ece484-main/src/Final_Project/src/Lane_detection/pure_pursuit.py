import numpy as np
from std_msgs.msg import String, Bool, Float32, Float64, Float64MultiArray
from ackermann_msgs.msg import AckermannDriveStamped, AckermannDrive

# ROS Headers
import rospy
class PP:
    def __init__(self):
        self.rate = rospy.Rate(60) # was 30
        self.look_ahead = 0.3 # 4
        self.wheelbase  = 0.325 # meters
        self.offset     = 0.017 # meters        

        self.ctrl_pub  = rospy.Publisher("/vesc/low_level/ackermann_cmd_mux/input/navigation", AckermannDriveStamped, queue_size=1)
        self.drive_msg = AckermannDriveStamped()
        self.drive_msg.header.frame_id = "f1tenth_control"
        self.drive_msg.drive.speed     = 0.6 # m/s, reference speed

        self.irl_measurement_X = 0.66   # measurement from irl from left to right
        self.irl_measurement_Y = 1.448   # measurement from irl from top to bottom
        self.pixel_to_irl_X = self.irl_measurement_X/640
        self.pixel_to_irl_Y = self.irl_measurement_Y/480
        self.prev_angle = 0

    def start_pp(self, xy, middleFlag):
        # not dead/sleep
        # while not rospy.is_shutdown():
        # calc look-ahead dist
        x = (xy[0] - (640/2)) * self.pixel_to_irl_X
        y = (480 - xy[1]) * self.pixel_to_irl_Y
        L = np.sqrt(x ** 2 + y ** 2)
        yaw = np.arctan(x/y)

        # --- tune this

        # if(not middleFlag):
        k = 1.0  # this almost made it around track with top_pixel_pos
        # else:
        #     k = .3
        # k = 0.835

        # ---

        angle = -np.arctan((k * 2 * self.wheelbase * np.sin(yaw)) / L) # might have to make sign change (* -1)

        return angle

 

def pure_pursuit():
    rospy.init_node('vicon_pp_node', anonymous=True)
    pp = PP()

    try:

        pp.start_pp()

    except rospy.ROSInterruptException:

        pass