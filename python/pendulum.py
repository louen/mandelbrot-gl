import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation

# https://chalkdustmagazine.com/features/the-magnetic-pendulum/

# Problem parameters
num_magnets = 3         # How many magnets
magnet_radius = 1.0     # Circle on which magnets are positioned
extents = 3.0           # Simulation drawing extents

magnetic_exponent = 4       # n such as magnetic force = K/(r^n )
magnetic_constant = 1     # multiplicative constant of magnetic force (in natural units)
friction = 0.2             # dissipative term
height = 0.5
delta_t = 0.01

magnet_positions = np.array([np.array([
    magnet_radius * np.cos(2 * i * np.pi / num_magnets),
    magnet_radius * np.sin(2 * i * np.pi / num_magnets)])
    for i in range(num_magnets)])

print(magnet_positions)


def update(pos, vel, dt):

   # assuming units so that gravity = 1 
    gravity =  -pos
    
    magnets = np.zeros(2)
    for p in magnet_positions:
        diff = p - pos
        d2 = diff.dot(diff) 
        v = magnetic_constant / ((d2 + height*height)**((magnetic_exponent+1)/2))

        magnets += v * diff
    

    # Semi implicit euler assuming dt = 1 

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


fig = plt.figure()
ax = plt.axes(xlim = [-extents, extents], ylim = [-extents, extents])

ax.scatter(x= magnet_positions[:,0], y = magnet_positions[:,1])


pos = np.array([1.0,1.0])
vel = np.zeros(2)
traj = np.array([pos])
points, = ax.plot([], [], "ro", ms=1)

def init():
    points.set_data([],[])
    return points,

def animate(i):
    global pos,vel,traj
    ax.scatter(pos[0], pos[1])
    pos,vel = update(pos,vel, delta_t)

    traj = np.append(traj, pos.reshape(1,2), 0)

    if (np.linalg.norm(vel) < 1e-3):
        pos = 4 * (np.random.random_sample((2,)) -  0.5* np.ones((2,)))
        vel = np.zeros(2)
        traj = np.array([pos])
    


    points.set_data(traj[:,0], traj[:,1])
    return points,

anim = animation.FuncAnimation(fig, animate, frames=1000, blit = True, interval = 10, init_func=init)

plt.show()
