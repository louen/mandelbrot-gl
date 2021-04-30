import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation

# https://chalkdustmagazine.com/features/the-magnetic-pendulum/

# Problem parameters
num_magnets = 3         # How many magnets
magnet_radius = 1.0     # Circle on which magnets are positioned
extents = 3.0           # Simulation drawing extents

magnetic_exponent = 4     # n such as magnetic force = K/(r^n )
magnetic_constant = 1     # multiplicative constant of magnetic force (in natural units)
friction = 0.1            # dissipative term
height = 0.5              # How high is the pendulum from the magnet
delta_t = 0.01

magnet_positions = np.array([np.array([
    magnet_radius * np.cos(2 * i * np.pi / num_magnets),
    magnet_radius * np.sin(2 * i * np.pi / num_magnets)])
    for i in range(num_magnets)])



def update(pos, vel, dt):

   # assuming units so that gravity = 1 
    gravity =  -pos
    
    magnets = np.zeros(2)
    for p in magnet_positions:
        diff = p - pos
        d2 = diff.dot(diff) 
        v = magnetic_constant / ((d2 + height*height)**((magnetic_exponent+1)/2))

        magnets += v * diff
    

    # Semi implicit euler 

    vel += dt * (gravity + magnets)
    vel *= (1.0 - dt*friction)
    pos += dt * (vel)

    return pos,vel


def simulate(starting_pos, starting_vel, dt, epsilon = 1e-7):
    pos = starting_pos
    vel = starting_vel

    trajectory = []

    while True:
        pos,vel = update(pos,vel, dt)
        if (np.linalg.norm(vel) < epsilon):
            break
        trajectory.append(pos)

    return trajectory

pos1 = np.array([2.0,0.8])
vel1 = np.zeros(2)
traj1 = np.array([pos1])

pos2 = np.array([2.02,0.8])
vel2 = np.zeros(2)
traj2 = np.array([pos2])
points1, = ax.plot([], [], "r,-", ms=1)
points2, = ax.plot([], [], "k,-", ms=1)

def init():
    points1.set_data([],[])
    points1.set_marker(',')
    points1.set_color('r')

    points2.set_data([],[])
    points2.set_marker(',')
    points2.set_color('k')
    return points1,points2


fig = plt.figure(figsize=(10,10))
ax = plt.axes(xlim = [-extents, extents], ylim = [-extents, extents])
ax.scatter(x= magnet_positions[:,0], y = magnet_positions[:,1])
def animate(i):
    global pos1,vel1,traj1
    global pos2,vel2,traj2
    #ax.scatter(pos[0], pos[1])
    pos1,vel1 = update(pos1,vel1, delta_t)
    pos2,vel2 = update(pos2,vel2, delta_t)

    traj1 = np.append(traj1, pos1.reshape(1,2), 0)
    traj2 = np.append(traj2, pos2.reshape(1,2), 0)

    # reset animation from a random point
    if (np.linalg.norm(vel) < 1e-2):
        pos = 4 * (np.random.random_sample((2,)) -  0.5* np.ones((2,)))
        vel = np.zeros(2)
        traj = np.array([pos])
    
    points1.set_data(traj1[:,0], traj1[:,1])
    points2.set_data(traj2[:,0], traj2[:,1])
    return points1,points2


anim = animation.FuncAnimation(fig, animate, frames=2000, blit = True, interval = 10, init_func=init)
plt.show()
