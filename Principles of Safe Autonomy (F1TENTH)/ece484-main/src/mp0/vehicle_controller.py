from enum import Enum, auto
import copy
from typing import List

class PedestrianMode(Enum):
    Normal=auto()

class VehicleMode(Enum):
    Normal = auto()
    Brake = auto()
    Accel = auto()
    HardBrake = auto()

class State:
    x: float 
    y: float 
    theta: float 
    v: float 
    agent_mode: VehicleMode 

    def __init__(self, x, y, theta, v, agent_mode: VehicleMode):
        pass 

def decisionLogic(ego: State, other: State):
    output = copy.deepcopy(ego)
    
    # TODO: Edit this part of decision logic
    
    hbdist = 10
    bdist = 20
    # ndist = 25
    adist = 35

    if ego.agent_mode != VehicleMode.HardBrake and other.dist < hbdist:
        output.agent_mode = VehicleMode.HardBrake
    elif ego.agent_mode != VehicleMode.Brake and other.dist < bdist and other.dist >= hbdist:
        output.agent_mode = VehicleMode.Brake
    elif ego.agent_mode != VehicleMode.Normal and other.dist >= bdist and other.dist <= adist:
        output.agent_mode = VehicleMode.Normal
    elif ego.agent_mode != VehicleMode.Accel and other.dist > adist:
        output.agent_mode = VehicleMode.Accel

    ###########################################

    assert other.dist > 2.0

    return output 