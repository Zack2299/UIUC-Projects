# read_waypoints.py
# ---------------
# Created by Alexander Mellas (amellas2@illinois.edu) on 11/03/2023


import matplotlib.pyplot as plt
import csv
import numpy as np
import yaml

# Function to extract the 'data' list from the YAML structure
def extract_data_list(yaml_string):
    yaml_data = yaml.safe_load(yaml_string)
    if yaml_data and "data" in yaml_data:
        return yaml_data["data"]
    return []


def get_waypoints(filename):
    # Initialize empty lists to store x, y, and yaw values
    x_values = []
    y_values = []
    yaw_values_rad = []
    yaw_values_deg = []

    # Read data from the CSV file
    with open(filename, 'r') as csvfile:
        csvreader = csv.reader(csvfile)
        next(csvreader) 

        for i, row in enumerate(csvreader):
            field_1_data = row[1]
            data_list = extract_data_list(field_1_data)

            if i % 10 == 0:  # Plots a point/arrow for every 10th point (can edit)
                x_value = float(data_list[0])
                y_value = float(data_list[1])
                yaw_value_deg = float(data_list[3])

                x_values.append(x_value)
                y_values.append(y_value)
                yaw_values_rad.append(np.radians(yaw_value_deg)) # radians

    # Create tuples from x and y values
    data_rad = list(zip(x_values, y_values, yaw_values_rad))
    data_deg = list(zip(x_values, y_values, yaw_values_deg))
    
    x_coords = [coord[0] for coord in data_rad]
    y_coords = [coord[1] for coord in data_rad]

    plt.figure(figsize=(8, 8))
    plt.plot(x_coords, y_coords, color='red')

    arrow_length_scale = 1.0

    # Plot arrows for each point based on yaw
    for x, y, yaw in zip(x_coords, y_coords, yaw_values_rad):
        dx = arrow_length_scale * np.cos(yaw)  # X-component of the arrow direction
        dy = arrow_length_scale * np.sin(yaw)  # Y-component of the arrow direction
        plt.arrow(x, y, dx, dy, head_width=0.01, head_length=0.01, fc='blue', ec='blue')

    plt.gca().set_aspect('equal', adjustable='box')
    plt.show()
    plt.savefig("Track_car_state")

    return


# TODO: Update file name if applicable
if __name__ == "__main__":
    get_waypoints('f1tenth_waypoints.csv')