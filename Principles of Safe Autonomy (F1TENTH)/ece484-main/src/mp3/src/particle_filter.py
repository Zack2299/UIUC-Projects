import numpy as np
from maze import Maze, Particle, Robot
import bisect
import rospy
from gazebo_msgs.msg import  ModelState
from gazebo_msgs.srv import GetModelState
import shutil
from std_msgs.msg import Float32MultiArray
from scipy.integrate import ode

import random

def vehicle_dynamics(t, vars, vr, delta):
    curr_x = vars[0]
    curr_y = vars[1] 
    curr_theta = vars[2]
    
    dx = vr * np.cos(curr_theta)
    dy = vr * np.sin(curr_theta)
    dtheta = delta
    return [dx,dy,dtheta]

class particleFilter:
    def __init__(self, bob, world, num_particles, sensor_limit, x_start, y_start):
        self.num_particles = num_particles  # The number of particles for the particle filter
        self.sensor_limit = sensor_limit    # The sensor limit of the sensor
        particles = list()

        ##### TODO:  #####
        # Modify the initial particle distribution to be within the top-right quadrant of the world, and compare the performance with the whole map distribution.
        for i in range(num_particles):

            # (Default) The whole map
            # x = np.random.uniform(0, world.width)
            # y = np.random.uniform(0, world.height)


            ## first quadrant
            x = np.random.uniform(world.width/2, world.width) 
            y = np.random.uniform(world.height/2, world.height)

            particles.append(Particle(x = x, y = y, maze = world, sensor_limit = sensor_limit))

        ###############

        self.particles = particles          # Randomly assign particles at the begining
        self.bob = bob                      # The estimated robot state
        self.world = world                  # The map of the maze
        self.x_start = x_start              # The starting position of the map in the gazebo simulator
        self.y_start = y_start              # The starting position of the map in the gazebo simulator
        self.modelStatePub = rospy.Publisher("/gazebo/set_model_state", ModelState, queue_size=1)
        self.controlSub = rospy.Subscriber("/gem/control", Float32MultiArray, self.__controlHandler, queue_size = 1)
        self.control = []                   # A list of control signal from the vehicle
        return

    def __controlHandler(self,data):
        """
        Description:
            Subscriber callback for /gem/control. Store control input from gem controller to be used in particleMotionModel.
        """
        tmp = list(data.data)
        self.control.append(tmp)

    def getModelState(self):
        """
        Description:
            Requests the current state of the polaris model when called
        Returns:
            modelState: contains the current model state of the polaris vehicle in gazebo
        """

        rospy.wait_for_service('/gazebo/get_model_state')
        try:
            serviceResponse = rospy.ServiceProxy('/gazebo/get_model_state', GetModelState)
            modelState = serviceResponse(model_name='polaris')
        except rospy.ServiceException as exc:
            rospy.loginfo("Service did not process request: "+str(exc))
        return modelState

    def weight_gaussian_kernel(self,x1, x2, std = 5000):
        if x1 is None: # If the robot recieved no sensor measurement, the weights are in uniform distribution.
            return 1./len(self.particles)
        else:
            tmp1 = np.array(x1)
            tmp2 = np.array(x2)
            return np.sum(np.exp(-((tmp2-tmp1) ** 2) / (2 * std)))


    def updateWeight(self, readings_robot):
        """
        Description:
            Update the weight of each particles according to the sensor reading from the robot 
        Input:
            readings_robot: List, contains the distance between robot and wall in [front, right, rear, left] direction.
        """

        ## TODO #####
        temp_array = np.array([])
        cumulative_sum = 0
        for particle in self.particles:
            sensor_data = particle.read_sensor()
            particle.weight = self.weight_gaussian_kernel(readings_robot, sensor_data) # closer particle is to robot, higher the weight based on gaussian stuff
            temp_array = np.append(temp_array, particle.weight)
            
        cumulative_sum = np.sum(temp_array)
        new_weights = temp_array / cumulative_sum
        
        # # # grab accumulative sum and divide all of them by it
        for count, particle in enumerate(self.particles):
            self.particles[count].weight = new_weights[count]

        
        ###############
        # pass

    def resampleParticle(self):
        """
        Description:
            Perform resample to get a new list of particles 
        """
        particles_new = list()

        ## TODO #####
        # in maze.py, in the particle class, there's a function called "FixedInvalidParticles", what it does.
        # because it gives noise, if it goes out, it'll fix it back in the bounds
        
        particle_sums = []

        # calc array of cumulative sum of weights
        for p in self.particles:
            particle_sums.append(p.weight)
        particle_sums = np.cumsum(particle_sums)

        # Repeat sampling until you have the desired number of samples.
        for i in range(len(self.particles)):
            # Randomly generate a number and determine which range in that cumulative weight array to which the number belongs
            random_num = random.uniform(0, 1)
            for count, particle_weight in enumerate(particle_sums):
                # The index of that range would correspond to the particle that should be created
                particle = self.particles[count]
                if particle_weight > random_num:
                    particles_new.append(Particle(particle.x, particle.y, particle.maze, particle.heading, particle.weight, particle.sensor_limit, True))
                    break
        ###############
        # print(len(self.particles), len(particles_new))
        self.particles = particles_new

    def particleMotionModel(self):
        """
        Description:
            Estimate the next state for each particle according to the control input from actual robot 
        """
        ## TODO #####
        # print(self.control)
        # from documentation
        timestep = 0.01
        for v, theta in self.control:
            for particle in self.particles:
                xDot = v*np.cos(particle.heading)
                yDot = v*np.sin(particle.heading)
                thetaDot = theta
                particle.x += xDot*timestep
                particle.y += yDot*timestep
                particle.heading += thetaDot*timestep
                # particle.fix_invalid_particles()
        self.control = []

            # rebound particles outside of the bounds

        ###############
        # pass


    def runFilter(self):
        """
        Description:
            Run PF localization
        """
        count = 0 
        while True:
            ## TODO: (i) Implement Section 3.2.2. (ii) Display robot and particles on map. (iii) Compute and save position/heading error to plot. #####
            self.world.show_particles(self.particles)
            self.world.show_robot(self.bob)
            self.world.show_estimated_location(self.particles)
            self.particleMotionModel()
            reading = self.bob.read_sensor()
            self.updateWeight(reading)
            self.resampleParticle()
            self.world.clear_objects()



            ###############
