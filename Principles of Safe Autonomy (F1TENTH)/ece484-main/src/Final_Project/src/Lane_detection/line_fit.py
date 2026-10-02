import numpy as np
import cv2
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import pickle
# from combined_thresh import combined_thresh
# from perspective_transform import perspective_transform

# feel free to adjust the parameters in the code if necessary

def line_fit(binary_warped):
    """
    Find and fit lane lines
    """
    # Assuming you have created a warped binary image called "binary_warped"
    # Take a histogram of the bottom half of the image
    histogram = np.sum(binary_warped[binary_warped.shape[0]//2:,:], axis=0)
    # Create an output image to draw on and visualize the result
    out_img = (np.dstack((binary_warped, binary_warped, binary_warped))*255).astype('uint8')
    img_with_points = (np.dstack((binary_warped, binary_warped, binary_warped))*255).astype('uint8')
    # Find the peak of the left and right halves of the histogram
    # These will be the starting point for the left and right lines
    midpoint = np.int(histogram.shape[0]/2)
    leftx_base = np.argmax(histogram[100:midpoint]) + 100
    rightx_base = np.argmax(histogram[midpoint:-100]) + midpoint

    # Choose the number of sliding windows
    nwindows = 9
    # Set height of windows
    window_height = np.int(binary_warped.shape[0]/nwindows)
    # Identify the x and y positions of all nonzero pixels in the image
    nonzero = binary_warped.nonzero()
    nonzeroy = np.array(nonzero[0])
    nonzerox = np.array(nonzero[1])
    # Current positions to be updated for each window
    leftx_current = leftx_base
    rightx_current = rightx_base
    # Set the width of the windows +/- margin
    margin = 100
    # Set minimum number of pixels found to recenter window
    minpix = 50
    # Create empty lists to receive left and right lane pixel indices
    left_lane_inds = []
    right_lane_inds = []

    #TAB:  

    # Step through the windows one by one
    for window in range(nwindows):
        # Identify window boundaries in x and y (and right and left)
        ## TODO
        # y-dim
        # print(window)
        y_top = window_height * (window)
        y_bottom = window_height * (window + 1)
       
        # x-dim
        x_left_start = leftx_base - margin
        x_left_end = leftx_base + margin

        x_right_start = rightx_base - margin
        x_right_end = rightx_base + margin

        ####
        # Draw the windows on the visualization image using cv2.rectangle()
        ## TODO
        cv2.rectangle(out_img, (x_left_start, y_top), (x_left_end, y_bottom), (0, 0, 255), 2)
        cv2.rectangle(out_img, (x_right_start, y_top), (x_right_end, y_bottom), (0,0, 255), 2)
        # cv2.imwrite("out_img.png", out_img)
        # cv2.imshow("out_img", out_img)

        ####
        # Identify the nonzero pixels in x and y within the window
        ## TODO
        nonzero_pixels_x_left = np.where((leftx_current - margin <= nonzerox) & (nonzerox <= leftx_current + margin)
                                    & (y_top <= nonzeroy) & (nonzeroy <= y_bottom))
        nonzero_pixels_x_right = np.where((rightx_current - margin <= nonzerox) & (nonzerox <= leftx_current + margin)
                                    & (y_top <= nonzeroy) & (nonzeroy <= y_bottom))

        ####
        # Append these indices to the lists
        ## TODO
        left_lane_inds.append(nonzero_pixels_x_left[0])
        right_lane_inds.append(nonzero_pixels_x_right[0])

        ####
        # If you found > minpix pixels, recenter next window on their mean position
        ## TODO
        if (len(left_lane_inds[0]) > minpix):
            leftx_current = sum(left_lane_inds[0])/len(left_lane_inds[0])
        if (len(right_lane_inds[0]) > minpix):
            rightx_current = sum(right_lane_inds[0])/len(right_lane_inds[0])
       
        ####
        pass


    if len(left_lane_inds[0]) == 0:
        avg_pixel_loc_x = 640/2
    else:
        avg_pixel_loc_x = sum(left_lane_inds[0])/len(left_lane_inds[0])
   
    # if(avg_pixel_loc_x < (640/2)-80):
    #   # print("Go Left")
    #   flag = -1
    # if(avg_pixel_loc_x > (640/2) + 80):
    #   # print("Go Right")
    #   flag = 1
    # if((640/2) - 80 <= avg_pixel_loc_x <= (640/2) + 80):
    #   # print("Go Forwards")
    #   flag = 0
    # # print("work plz")

    # flag = 1



    # Concatenate the arrays of indices
    left_lane_inds = np.concatenate(left_lane_inds)
    right_lane_inds = np.concatenate(right_lane_inds)

    # Extract left and right line pixel positions
    leftx = nonzerox[left_lane_inds]
    lefty = nonzeroy[left_lane_inds]
    # print(leftx)
    # print(lefty)
    rightx = nonzerox[right_lane_inds]
    righty = nonzeroy[right_lane_inds]

    # Fit a second order polynomial to each using np.polyfit()
    # If there isn't a good fit, meaning any of leftx, lefty, rightx, and righty are empty,
    # the second order polynomial is unable to be sovled.
    # Thus, it is unable to detect edges.
    try:
    ## TODO
        left_fit = np.polyfit(lefty, leftx, 2)
        right_fit = np.polyfit(righty, rightx, 2)
       
        # if flag == -1:
        #   print("left")
        # if flag == 0:
        #   print("forward")
        # if flag == 1:
        #   print("right")


    ####
    except TypeError:
        print("Unable to detect lanes")
        return None


    # Return a dict of relevant variables
    ret = {}
    ret['left_fit'] = left_fit
    ret['right_fit'] = right_fit
    ret['nonzerox'] = nonzerox
    ret['nonzeroy'] = nonzeroy
    ret['out_img'] = out_img
    ret['left_lane_inds'] = left_lane_inds
    # print(left_lane_inds)
    ret['right_lane_inds'] = right_lane_inds
    ret['test'] = avg_pixel_loc_x
    ret['allX'] = nonzerox[right_lane_inds]
    ret['allY'] = nonzeroy[right_lane_inds]
    ret['firsty'] = nonzeroy[right_lane_inds][0]
    ret['lasty'] = nonzeroy[right_lane_inds][-1]
    # print(np.bincount(lefty)[0])
    # ret['numPixels'] = np.bincount(lefty)[0]

    cond = (nonzeroy[right_lane_inds] == nonzeroy[right_lane_inds][0])
    cond2 = (nonzeroy[right_lane_inds] == nonzeroy[right_lane_inds][-1])

    idx = int(len(nonzeroy[right_lane_inds])/2)
    middleY = nonzeroy[right_lane_inds][idx]

    cond3 = (nonzeroy[right_lane_inds] == nonzeroy[right_lane_inds][idx])

    ret['first_Y_pixels'] = nonzeroy[right_lane_inds][cond]
    ret['last_Y_pixels'] = nonzeroy[right_lane_inds][cond2]
    ret['middle_Y_pixels'] = cond3
   
    ret['xpixels'] = nonzerox[right_lane_inds]

    averageXtop, averageXBottom, middleX = get_ref_points(ret['first_Y_pixels'], ret['last_Y_pixels'], ret['xpixels'], ret['middle_Y_pixels'])
    cv2.circle(img_with_points, (int(averageXtop), righty[0]), 2, (0,0, 255), 2)
    cv2.circle(img_with_points, (int(middleX), righty[int(len(righty)/2)]), 2, (0,255, 0), 2)
    # cv2.imwrite("birds_eye_with_ref.png", img_with_points)

    return ret


def tune_fit(binary_warped, left_fit, right_fit):
    """
    Given a previously fit line, quickly try to find the line based on previous lines
    """
    # Assume you now have a new warped binary image
    # from the next frame of video (also called "binary_warped")
    # It's now much easier to find line pixels!
    nonzero = binary_warped.nonzero()
    nonzeroy = np.array(nonzero[0])
    nonzerox = np.array(nonzero[1])
    margin = 100
    left_lane_inds = ((nonzerox > (left_fit[0]*(nonzeroy**2) + left_fit[1]*nonzeroy + left_fit[2] - margin)) & (nonzerox < (left_fit[0]*(nonzeroy**2) + left_fit[1]*nonzeroy + left_fit[2] + margin)))
    right_lane_inds = ((nonzerox > (right_fit[0]*(nonzeroy**2) + right_fit[1]*nonzeroy + right_fit[2] - margin)) & (nonzerox < (right_fit[0]*(nonzeroy**2) + right_fit[1]*nonzeroy + right_fit[2] + margin)))
    img_with_points = (np.dstack((binary_warped, binary_warped, binary_warped))*255).astype('uint8')

    # Again, extract left and right line pixel positions
    leftx = nonzerox[left_lane_inds]
    lefty = nonzeroy[left_lane_inds]
    rightx = nonzerox[right_lane_inds]
    righty = nonzeroy[right_lane_inds]

    # If we don't find enough relevant points, return all None (this means error)
    min_inds = 10
    if lefty.shape[0] < min_inds or righty.shape[0] < min_inds:
        return None

    # Fit a second order polynomial to each
    left_fit = np.polyfit(lefty, leftx, 2)
    right_fit = np.polyfit(righty, rightx, 2)
    # Generate x and y values for plotting
    ploty = np.linspace(0, binary_warped.shape[0]-1, binary_warped.shape[0] )
    left_fitx = left_fit[0]*ploty**2 + left_fit[1]*ploty + left_fit[2]
    right_fitx = right_fit[0]*ploty**2 + right_fit[1]*ploty + right_fit[2]


    # FINAL PROJECT LOGIC
    if len(leftx) == 0:
        avg_pixel_loc_x = 640/2
    else:
        avg_pixel_loc_x = sum(leftx)/len(leftx)

    # Return a dict of relevant variables
    ret = {}
    ret['left_fit'] = left_fit
    ret['right_fit'] = right_fit
    ret['nonzerox'] = nonzerox
    ret['nonzeroy'] = nonzeroy
    ret['left_lane_inds'] = left_lane_inds

    ret['right_lane_inds'] = right_lane_inds
    ret['test'] = avg_pixel_loc_x
    ret['allX'] = nonzerox[right_lane_inds]
    ret['allY'] = nonzeroy[right_lane_inds]
    ret['firsty'] = nonzeroy[right_lane_inds][0]
    ret['lasty'] = nonzeroy[right_lane_inds][-1]

    cond = (nonzeroy[right_lane_inds] == nonzeroy[right_lane_inds][0])
    cond2 = (nonzeroy[right_lane_inds] == nonzeroy[right_lane_inds][-1])

    idx = int(len(nonzeroy[right_lane_inds])/2)
    middleY = nonzeroy[right_lane_inds][idx]
    middle_mask = np.zeros_like(nonzeroy[right_lane_inds])
    cond3 = (nonzeroy[right_lane_inds] == nonzeroy[right_lane_inds][idx])
    # print(cond3)

    ret['first_Y_pixels'] = nonzeroy[right_lane_inds][cond]
    ret['last_Y_pixels'] = nonzeroy[right_lane_inds][cond2]
    ret['middle_Y_pixels'] = cond3
   
    ret['xpixels'] = nonzerox[right_lane_inds]

    averageXtop, averageXBottom, middleX = get_ref_points(ret['first_Y_pixels'], ret['last_Y_pixels'], ret['xpixels'], ret['middle_Y_pixels'])
    cv2.circle(img_with_points, (int(averageXtop), righty[0]), 2, (0,0, 255), 2)
    cv2.circle(img_with_points, (int(middleX), righty[int(len(righty)/2)]), 2, (0,255, 0), 2)
    # cv2.imwrite("birds_eye_with_ref.png", img_with_points)

    return ret

def get_ref_points(first_Y_pixels, last_Y_pixels, xpixels, middle_Y_pixels):
    x_coordsTop = np.array([])
    x_coordsBottom = np.array([])
    for i in range(len(first_Y_pixels)):
        cur_coord = xpixels[i]
        x_coordsTop = np.append(x_coordsTop, cur_coord)
    averageXtop = sum(x_coordsTop)/len(x_coordsTop)
        #gets average x coord of all white pixels that have bottom most y_coord
    for i in range(len(last_Y_pixels)):
        cur_coord = xpixels[-i]
        x_coordsBottom = np.append(x_coordsBottom, cur_coord)
    averageXBottom = sum(x_coordsBottom)/len(x_coordsBottom)

    # idx = int(len(xpixels)/2)
    # print(xpixels[middle_Y_pixels])
    # print("middle X: ", xpixels[middle_Y_pixels])
    # print("middle: ", middle_Y_pixels)

    middleX_idx = int(sum(xpixels[middle_Y_pixels])/len(xpixels[middle_Y_pixels]))
    return averageXtop, averageXBottom, middleX_idx

def viz1(binary_warped, ret, save_file=None):
    """
    Visualize each sliding window location and predicted lane lines, on binary warped image
    save_file is a string representing where to save the image (if None, then just display)
    """
    # Grab variables from ret dictionary
    left_fit = ret['left_fit']
    right_fit = ret['right_fit']
    nonzerox = ret['nonzerox']
    nonzeroy = ret['nonzeroy']
    out_img = ret['out_img']
    left_lane_inds = ret['left_lane_inds']
    right_lane_inds = ret['right_lane_inds']

    # Generate x and y values for plotting
    ploty = np.linspace(0, binary_warped.shape[0]-1, binary_warped.shape[0] )
    left_fitx = left_fit[0]*ploty**2 + left_fit[1]*ploty + left_fit[2]
    right_fitx = right_fit[0]*ploty**2 + right_fit[1]*ploty + right_fit[2]

    out_img[nonzeroy[left_lane_inds], nonzerox[left_lane_inds]] = [255, 0, 0]
    out_img[nonzeroy[right_lane_inds], nonzerox[right_lane_inds]] = [0, 0, 255]
    plt.imshow(out_img)
    plt.plot(left_fitx, ploty, color='yellow')
    plt.plot(right_fitx, ploty, color='yellow')
    plt.xlim(0, 1280)
    plt.ylim(720, 0)
    if save_file is None:
        plt.show()
    else:
        plt.savefig(save_file)
    plt.gcf().clear()


def bird_fit(binary_warped, ret, save_file=None):
    """
    Visualize the predicted lane lines with margin, on binary warped image
    save_file is a string representing where to save the image (if None, then just display)
    """
    # Grab variables from ret dictionary
    left_fit = ret['left_fit']
    right_fit = ret['right_fit']
    nonzerox = ret['nonzerox']
    nonzeroy = ret['nonzeroy']
    left_lane_inds = ret['left_lane_inds']
    right_lane_inds = ret['right_lane_inds']
    img_with_points = (np.dstack((binary_warped, binary_warped, binary_warped))*255).astype('uint8')
    righty = ret['allY']

    averageXtop, averageXBottom, middleX = get_ref_points(ret['first_Y_pixels'], ret['last_Y_pixels'], ret['xpixels'], ret['middle_Y_pixels'])
    cv2.circle(img_with_points, (int(averageXtop), righty[0]), 10, (0,0, 255), 5)
    cv2.circle(img_with_points, (int(middleX), righty[int(len(righty)/2)]), 2, (0,255, 0), 2)
    # cv2.imwrite("birds_eye_with_ref.png", img_with_points)

    # Create an image to draw on and an image to show the selection window
    out_img = (np.dstack((binary_warped, binary_warped, binary_warped))*255).astype('uint8')
    window_img = np.zeros_like(out_img)
    # Color in left and right line pixels
    out_img[nonzeroy[left_lane_inds], nonzerox[left_lane_inds]] = [255, 0, 0]
    out_img[nonzeroy[right_lane_inds], nonzerox[right_lane_inds]] = [255, 0, 0]

    # Generate x and y values for plotting
    ploty = np.linspace(0, binary_warped.shape[0]-1, binary_warped.shape[0])
    left_fitx = left_fit[0]*ploty**2 + left_fit[1]*ploty + left_fit[2]
    right_fitx = right_fit[0]*ploty**2 + right_fit[1]*ploty + right_fit[2]

    # Generate a polygon to illustrate the search window area
    # And recast the x and y points into usable format for cv2.fillPoly()
    margin = 100  # NOTE: Keep this in sync with *_fit()
    left_line_window1 = np.array([np.transpose(np.vstack([left_fitx-margin, ploty]))])
    left_line_window2 = np.array([np.flipud(np.transpose(np.vstack([left_fitx+margin, ploty])))])
    left_line_pts = np.hstack((left_line_window1, left_line_window2))
    # print(left_line_pts)
    right_line_window1 = np.array([np.transpose(np.vstack([right_fitx-margin, ploty]))])
    right_line_window2 = np.array([np.flipud(np.transpose(np.vstack([right_fitx+margin, ploty])))])
    right_line_pts = np.hstack((right_line_window1, right_line_window2))


    # Draw the lane onto the warped blank image
    cv2.fillPoly(window_img, np.int_([left_line_pts]), (0,0, 255))
    cv2.fillPoly(window_img, np.int_([right_line_pts]), (0,0, 255))
    result = cv2.addWeighted(out_img, 1, window_img, 0.3, 0)

    # plt.imshow(result)
    # plt.plot(left_fitx, ploty, color='yellow')
    # plt.plot(right_fitx, ploty, color='yellow')
    # plt.xlim(0, 1280)
    # plt.ylim(720, 0)

    # cv2.imshow('bird',result)
    # cv2.imwrite('bird_from_cv2.png', result)

    # if save_file is None:
    #   plt.show()
    # else:
    #   plt.savefig(save_file)
    # plt.gcf().clear()

    return img_with_points, result


def final_viz(undist, left_fit, right_fit, m_inv):
    """
    Final lane line prediction visualized and overlayed on top of original image
    """
    # Generate x and y values for plotting
    ploty = np.linspace(0, undist.shape[0]-1, undist.shape[0])
    left_fitx = left_fit[0]*ploty**2 + left_fit[1]*ploty + left_fit[2]
    right_fitx = right_fit[0]*ploty**2 + right_fit[1]*ploty + right_fit[2]

    # Create an image to draw the lines on
    #warp_zero = np.zeros_like(warped).astype(np.uint8)
    #color_warp = np.dstack((warp_zero, warp_zero, warp_zero))
    color_warp = np.zeros((720, 1280, 3), dtype='uint8')  # NOTE: Hard-coded image dimensions

    # Recast the x and y points into usable format for cv2.fillPoly()
    pts_left = np.array([np.transpose(np.vstack([left_fitx, ploty]))])
    pts_right = np.array([np.flipud(np.transpose(np.vstack([right_fitx, ploty])))])
    pts = np.hstack((pts_left, pts_right))

    # Draw the lane onto the warped blank image
    cv2.fillPoly(color_warp, np.int_([pts]), (0,255, 0))

    # Warp the blank back to original image space using inverse perspective matrix (Minv)
    newwarp = cv2.warpPerspective(color_warp, m_inv, (undist.shape[1], undist.shape[0]))
    # Combine the result with the original image
    # Convert arrays to 8 bit for later cv to ros image transfer
    undist = np.array(undist, dtype=np.uint8)
    newwarp = np.array(newwarp, dtype=np.uint8)
    result = cv2.addWeighted(undist, 1, newwarp, 0.3, 0)

    return result