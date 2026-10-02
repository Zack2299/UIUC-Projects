import numpy as np
import time


class ZACcontroller:
    def __init__(self, Kz, Ka, Kc):
        # gain for each term. might need to split into tuples but for now just one term
        self.Kz = Kz
        self.Ka = Ka
        self.Kc = Kc
        # cte = cross-track error, ye = yaw error
        self.prev_cte_error = 0
        self.prev_ye_error = 0
        self.cte_cumsum = 0 
        self.ye_cumsum = 0
        self.ZAC_signal = (0, 0)
        # self.prev_cte_time = time.time()

    # target state is cross-track error and yaw error we want (I think) and then cur_state is what we are currently at
    # our very primitive implementatin for 11/9 should use a target of small cross cte and ye like (0,0) or something
    # and then cur_state we just guess based on location of average pixel (larger if closer to edges of screen)
    # (we will refine this later by actually determining cte and ye)
    def compute_sig(self, target_state, cur_state, theta, L):
        # calculate error between the target state and cur_state
        pixel_to_irl = 1800
        cte_error = (((target_state[0] - cur_state[0])) + L*np.sin(theta)) / pixel_to_irl

        # grab start
        # start = time.time()
        # elapsed_time = start - self.prev_cte_time



        # Z - Zoom in errors (proportional term)
        Z = (self.Kz * cte_error, self.Kz)

        # A - Accumulate (integral term)
        self.cte_cumsum += cte_error
        A = (self.Ka * self.cte_cumsum, self.Ka * self.ye_cumsum)

        # C - Calculus (derivative term)
        C = (self.Kc * ((cte_error - self.prev_cte_error) / 1), self.Kc)

        ZAC = (Z[0] + A[0] + C[0], Z[1] + A[1] + C[1])
        # JOSH'S = (M[0] + O[0] + M[0])

        self.prev_cte_error = cte_error
        # self.prev_cte_time = start

        # print("change in time: ", elapsed_time)

        self.ZAC_signal = ZAC # signal to use for decision-making on what angle (and speed) should be