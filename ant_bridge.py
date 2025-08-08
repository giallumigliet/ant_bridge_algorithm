import random
import time
import os

# --- Simulation Constants ---
NUM_ANT = 15
NUM_DRONE_ANT = 2
ENV_X = 9
ENV_Y = 9
ANT_FORCE = 1  # Max MOVING ants a STILL ant can support
PRINTF_SPEED = 50000  # Microseconds
PERC_NO_OBSTACLES = 7  # 7 out of 10, so 70% probability of ground

# --- Ant States ---
MOVING = 0
STILL = 1
WARNING = 2

# --- ANSI Color Codes for Console ---
COLOR_ORANGE = "\033[38;5;208m"
COLOR_BLUE = "\033[34m"
COLOR_GREEN = "\033[32m"
COLOR_RED = "\033[31m"
COLOR_BOLD_RED = "\033[1;31m"
COLOR_RESET = "\033[0m"

class Ant:
    """Represents an ant with its ID, position, and state."""
    def __init__(self, id_type=0, pos_x=0, pos_y=0, state=MOVING):
        self.id_type = id_type  # 0=worker, 1=drone
        self.position_x = pos_x
        self.position_y = pos_y
        self.state = state

class Position:
    """Represents a coordinate (x, y)."""
    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y

def print_pheromone_map(environment, pheromone_map, returned, best_path):
    """Prints the pheromone map."""
    print("\nPheromone Map:")
    for iY in range(ENV_Y):
        for iX in range(ENV_X):
            value = 0
            if returned:
                value = pheromone_map[best_path][iX][iY]
            else:
                value = sum(pheromone_map[i][iX][iY] for i in range(NUM_DRONE_ANT))

            if value > 0:
                print(f"{COLOR_BLUE}{value:2d}  {COLOR_RESET}", end="")
            else:
                if environment[iX][iY] == 3: # Target
                    print(f"{COLOR_ORANGE} T  {COLOR_RESET}", end="")
                elif environment[iX][iY] == 4: # Nest
                    print(f"{COLOR_BLUE} N  {COLOR_RESET}", end="")
                else:
                    print(f"{value:2d}  ", end="")
        print()

def print_terrain_with_ants(ants, environment, pheromone_map, returned, best_path):
    """Clears the console and prints the current state of the simulation."""
    os.system('cls' if os.name == 'nt' else 'clear')
    
    occupations = [[0 for _ in range(ENV_Y)] for _ in range(ENV_X)]
    for ant in ants:
        occupations[ant.position_x][ant.position_y] += 1

    print("\nEnvironment with ants:")
    for iY in range(ENV_Y):
        for iX in range(ENV_X):
            if occupations[iX][iY] > 0:
                print(f"{COLOR_BLUE}{occupations[iX][iY]:2d}  {COLOR_RESET}", end="")
            else:
                if environment[iX][iY] == 1: # Ground
                    print(f"{COLOR_GREEN} X  {COLOR_RESET}", end="")
                elif environment[iX][iY] == 2: # Obstacle
                    print(f"{COLOR_RED} =  {COLOR_RESET}", end="")
                elif environment[iX][iY] == 3: # Target
                    print(f"{COLOR_ORANGE} T  {COLOR_RESET}", end="")
                elif environment[iX][iY] == 4: # Nest
                    print(f"{COLOR_BLUE} N  {COLOR_RESET}", end="")
                else: # Hole
                    print(" .  ", end="")
        print()

    print("\nEnvironment:")
    for iY in range(ENV_Y):
        for iX in range(ENV_X):
            if environment[iX][iY] == 1:
                print(f"{COLOR_GREEN} X  {COLOR_RESET}", end="")
            elif environment[iX][iY] == 2:
                print(f"{COLOR_RED} =  {COLOR_RESET}", end="")
            elif environment[iX][iY] == 3:
                print(f"{COLOR_ORANGE} T  {COLOR_RESET}", end="")
            elif environment[iX][iY] == 4:
                print(f"{COLOR_BLUE} N  {COLOR_RESET}", end="")
            else:
                print(" .  ", end="")
        print()
        
    print_pheromone_map(environment, pheromone_map, returned, best_path)
    time.sleep(PRINTF_SPEED / 1_000_000.0)



def over_weight_detection(ants, current_ant):
    """Detects if a STILL ant is supporting too much weight."""
    counter_moving = 0
    counter_still = 0
    for ant in ants:
        if ant.position_x == current_ant.position_x and ant.position_y == current_ant.position_y:
            if ant.state == MOVING:
                counter_moving += 1
            elif ant.state in [STILL, WARNING]:
                counter_still += 1
    return counter_moving > (counter_still * ANT_FORCE)

def no_weight_detection(ants, current_ant):
    """Detects if there are no MOVING ants on top of a STILL ant."""
    for ant in ants:
        if ant.state == MOVING and ant.position_x == current_ant.position_x and ant.position_y == current_ant.position_y:
            return False
    return True

def warning_detection(ants, pos_x, pos_y):
    """Detects if there is an ant in a WARNING state at a given position."""
    for ant in ants:
        if ant.state == WARNING and ant.position_x == pos_x and ant.position_y == pos_y:
            return True
    return False

def ant_bridge_present(ants, current_ant_index, check_x, check_y):
    """Checks if a bridge ant is at a given position."""
    for i, ant in enumerate(ants):
        if i != current_ant_index and ant.state in [STILL, WARNING] and ant.position_x == check_x and ant.position_y == check_y:
            return True
    return False

def hole_detection(ants, current_ant_index, next_step, environment):
    """Detects if the next step is a hole without a bridge."""
    if 0 <= next_step.x < ENV_X and 0 <= next_step.y < ENV_Y:
        is_hole = environment[next_step.x][next_step.y] == 0
        return is_hole and not ant_bridge_present(ants, current_ant_index, next_step.x, next_step.y)
    return False

def bridge_detection(ants, current_ant_index):
    """Checks if the current ant is a necessary part of a bridge."""
    ant = ants[current_ant_index]
    x, y = ant.position_x, ant.position_y
    
    neighbors = [(x, y - 1), (x, y + 1), (x - 1, y), (x + 1, y)]
    for nx, ny in neighbors:
        if 0 <= nx < ENV_X and 0 <= ny < ENV_Y:
            if ant_bridge_present(ants, current_ant_index, nx, ny) and not ant_bridge_present(ants, current_ant_index, x, y):
                return True
    return False


def print_states(ants):
    """Prints the final states of the ants."""
    print("FINAL STATES:")
    for i, ant in enumerate(ants):
        if i >= NUM_DRONE_ANT:
            state_str = "MOVING" if ant.state == MOVING else "STILL" if ant.state == STILL else "WARNING"
            print(f"ant {i}, position: ({ant.position_x},{ant.position_y}) in state: {state_str}")

def choose_nest(ants, environment, pheromone_map):
    """Allows the user to choose the nest's position."""
    print("INFO: nest and target can be set only on the ground X!!")
    while True:
        try:
            x = int(input("Select X coordinate for the NEST: "))
            y = int(input("Select Y coordinate for the NEST: "))
            if environment[x][y] == 1:
                environment[x][y] = 4  # 4 = Nest
                for ant in ants:
                    ant.position_x = x
                    ant.position_y = y
                print(f"OK nest in ({x}, {y})")
                print_terrain_with_ants(ants, environment, pheromone_map, False, 0)
                return Position(x, y)
            else:
                print("Invalid position. Please choose a ground cell ('X').")
        except (ValueError, IndexError):
            print("Invalid input. Please enter valid coordinates.")

def target_smell_intensity_obstacles(target_smell_map, x, y, environment):
    """Calculates the smell intensity from the target, avoiding obstacles."""
    q = [(x, y)]
    
    for iX in range(ENV_X):
        for iY in range(ENV_Y):
            target_smell_map[iX][iY] = 0
            
    max_intensity = (ENV_X + ENV_Y) * (20 - PERC_NO_OBSTACLES) / 10
    target_smell_map[x][y] = max_intensity
    
    head = 0
    while head < len(q):
        curr_x, curr_y = q[head]
        head += 1
        
        current_intensity = target_smell_map[curr_x][curr_y]
        if current_intensity <= 1:
            continue

        neighbors = [(curr_x - 1, curr_y), (curr_x + 1, curr_y), (curr_x, curr_y - 1), (curr_x, curr_y + 1)]
        for next_x, next_y in neighbors:
            if 0 <= next_x < ENV_X and 0 <= next_y < ENV_Y and environment[next_x][next_y] != 2 and target_smell_map[next_x][next_y] == 0:
                target_smell_map[next_x][next_y] = current_intensity - 1
                q.append((next_x, next_y))


def lay_pheromone(selected_map, pheromone_map, x, y):
    """Lays down pheromones on the map."""
    pheromone_map[selected_map][x][y] += 1

def choose_target(ants, environment, target_smell_map, pheromone_map):
    """Allows the user to choose the target's position."""
    while True:
        try:
            x = int(input("Select X coordinate for the TARGET: "))
            y = int(input("Select Y coordinate for the TARGET: "))
            if environment[x][y] == 1:
                environment[x][y] = 3  # 3 = Target
                print(f"OK target in ({x}, {y})")
                target_smell_intensity_obstacles(target_smell_map, x, y, environment)

                print("\n\nSmell Intensity Map:")
                for iY in range(ENV_Y):
                    for iX in range(ENV_X):
                        cell_val = target_smell_map[iX][iY]
                        if environment[iX][iY] == 3:
                            print(f"{COLOR_ORANGE}{cell_val:2.0f}  {COLOR_RESET}", end="")
                        elif environment[iX][iY] == 2:
                             print(f"{COLOR_RED}{cell_val:2.0f}  {COLOR_RESET}", end="")
                        else:
                            print(f"{cell_val:2.0f}  ", end="")
                    print()
                return Position(x, y)
            else:
                print("Invalid position. Please choose a ground cell ('X').")
        except (ValueError, IndexError):
            print("Invalid input. Please enter valid coordinates.")

def run_simulation():
    """Waits for user confirmation to start the simulation."""
    while True:
        run = input("\nPress 1 to run the simulation... ")
        if run == '1':
            return True
        print("Wrong input, simulation not run!!")

def smell_direction(x, y, target_smell_map, environment):
    """Chooses the direction with the highest smell intensity."""
    next_pos = Position(x, y)
    max_val = -1 # Start with a value lower than any possible smell
    
    possible_moves = []
    
    # Check all neighbors
    neighbors = [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    random.shuffle(neighbors) # Randomize to break ties

    for nx, ny in neighbors:
        if 0 <= nx < ENV_X and 0 <= ny < ENV_Y and environment[nx][ny] != 2:
            if target_smell_map[nx][ny] > max_val:
                max_val = target_smell_map[nx][ny]
                possible_moves = [(nx, ny)]
            elif target_smell_map[nx][ny] == max_val:
                possible_moves.append((nx, ny))

    if possible_moves:
        chosen_x, chosen_y = random.choice(possible_moves)
        next_pos.x = chosen_x
        next_pos.y = chosen_y
        
    return next_pos


def smell_pheromone(selected_map, x, y, pheromone_map, prev_pos):
    """Follows the pheromone trail, trying not to go backward."""
    next_pos = Position(x, y)
    max_phero = -1
    
    possible_moves = []
    
    neighbors = [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    random.shuffle(neighbors)
    
    for nx, ny in neighbors:
        is_not_previous = (nx != prev_pos.x or ny != prev_pos.y)
        if 0 <= nx < ENV_X and 0 <= ny < ENV_Y and is_not_previous:
            if pheromone_map[selected_map][nx][ny] > max_phero:
                max_phero = pheromone_map[selected_map][nx][ny]
                possible_moves = [(nx, ny)]
            elif pheromone_map[selected_map][nx][ny] == max_phero and max_phero > 0:
                possible_moves.append((nx,ny))

    if possible_moves:
        chosen_x, chosen_y = random.choice(possible_moves)
        next_pos.x = chosen_x
        next_pos.y = chosen_y
    else: # If stuck, allow going back
        max_phero = -1
        for nx, ny in neighbors:
            if 0 <= nx < ENV_X and 0 <= ny < ENV_Y:
                if pheromone_map[selected_map][nx][ny] > max_phero:
                   max_phero = pheromone_map[selected_map][nx][ny]
                   next_pos.x = nx
                   next_pos.y = ny

    prev_pos.x = x
    prev_pos.y = y
    return next_pos


def main():
    """Main function that runs the simulation."""
    # --- Initialization ---
    ants = [Ant() for _ in range(NUM_ANT)]
    holes_detected = [0] * NUM_DRONE_ANT

    for i in range(NUM_DRONE_ANT):
        ants[i].id_type = 1  # 1 = Drone

    environment = [[0] * ENV_Y for _ in range(ENV_X)]
    for ix in range(ENV_X):
        for iy in range(ENV_Y):
            # 0=hole, 1=ground, 2=obstacle
            env_type = 1 if random.randint(0, 1) == 1 else 0
            if env_type == 1 and random.randint(1, 10) > PERC_NO_OBSTACLES:
                environment[ix][iy] = 2
            else:
                environment[ix][iy] = env_type

    target_smell_map = [[0] * ENV_Y for _ in range(ENV_X)]
    pheromone_map = [[[0] * ENV_Y for _ in range(ENV_X)] for _ in range(NUM_DRONE_ANT)]
    
    # --- Environment Setup ---
    best_path = 0
    returned = False
    nest = choose_nest(ants, environment, pheromone_map)
    target = choose_target(ants, environment, target_smell_map, pheromone_map)
    
    prev_pos = [Position(nest.x, nest.y) for _ in range(NUM_ANT)]

    if not run_simulation():
        return

    # === PHASE 1: DRONE ANTS SEARCHING FOR FOOD ===
    print(f"{COLOR_BOLD_RED}\nFASE 1: DRONE ANTS SEARCHING FOR FOOD{COLOR_RESET}")
    found = False
    ant_found = 0
    while not found:
        current_ant_idx = random.randrange(NUM_DRONE_ANT)
        ant = ants[current_ant_idx]

        if ant.position_x != target.x or ant.position_y != target.y:
            next_step = smell_direction(ant.position_x, ant.position_y, target_smell_map, environment)
            lay_pheromone(current_ant_idx, pheromone_map, ant.position_x, ant.position_y)
            
            ant.position_x = next_step.x
            ant.position_y = next_step.y

            if environment[ant.position_x][ant.position_y] == 0:
                holes_detected[current_ant_idx] += 1
            
            if ant.position_x == target.x and ant.position_y == target.y:
                ant_found += 1
            
            print_terrain_with_ants(ants, environment, pheromone_map, returned, best_path)
            print(f"\nAnt {current_ant_idx} (Drone) moves to ({next_step.x}, {next_step.y})")
        
        if ant_found == NUM_DRONE_ANT:
            found = True

    # === PHASE 2: DRONE ANTS RETURNING HOME ===
    print(f"{COLOR_BOLD_RED}\nFASE 2: DRONE ANTS RETURNING HOME BY FOLLOWING THEIR OWN PHEROMONE PATH{COLOR_RESET}")
    time.sleep(4)
    for i in range(NUM_DRONE_ANT):
        prev_pos[i] = Position(ants[i].position_x, ants[i].position_y)
        
    ant_returned = 0
    while not returned:
        current_ant_idx = random.randrange(NUM_DRONE_ANT)
        ant = ants[current_ant_idx]

        if ant.position_x != nest.x or ant.position_y != nest.y:
            lay_pheromone(current_ant_idx, pheromone_map, ant.position_x, ant.position_y)
            next_step = smell_pheromone(current_ant_idx, ant.position_x, ant.position_y, pheromone_map, prev_pos[current_ant_idx])
            ant.position_x = next_step.x
            ant.position_y = next_step.y

            if ant.position_x == nest.x and ant.position_y == nest.y:
                ant_returned += 1
            
            print_terrain_with_ants(ants, environment, pheromone_map, returned, best_path)
            print(f"\nAnt {current_ant_idx} (Drone) returns to ({next_step.x}, {next_step.y})")

        if ant_returned == NUM_DRONE_ANT:
            returned = True

    # --- Best Path Selection ---
    min_holes = ENV_X * ENV_Y
    for i in range(NUM_DRONE_ANT):
        if holes_detected[i] < min_holes:
            min_holes = holes_detected[i]
            best_path = i
        print(f"\nThe ant {i}, detected {holes_detected[i]} holes")
        print_pheromone_map(environment, pheromone_map, True, i)
        
    print(f"\nTHE BEST PATH IS OF THE ANT {best_path}, WITH {min_holes} HOLES")
    time.sleep(8)
    
    # === PHASE 3: WORKER ANTS FOLLOWING THE PATH ===
    print(f"{COLOR_BOLD_RED}\nFASE 3: WORKER ANTS FOLLOWING PHEROMONE PATH FOR THE TARGET{COLOR_RESET}")
    time.sleep(4)
    for i in range(NUM_ANT):
        prev_pos[i] = Position(ants[i].position_x, ants[i].position_y)

    moving_presence = True
    while moving_presence:
        current_ant_idx = NUM_DRONE_ANT + random.randrange(NUM_ANT - NUM_DRONE_ANT)
        ant = ants[current_ant_idx]
        
        print_terrain_with_ants(ants, environment, pheromone_map, returned, best_path)
        print(f"\nSelected worker ant: {current_ant_idx}")

        if ant.state == WARNING:
            print(f"{COLOR_RED} WAAAAAAAAAAAAARNING!{COLOR_RESET}")
            time.sleep(0.5)

        if ant.state == MOVING and (ant.position_x != target.x or ant.position_y != target.y):
            if warning_detection(ants, ant.position_x, ant.position_y):
                ant.state = STILL
            else:
                next_step = smell_pheromone(best_path, ant.position_x, ant.position_y, pheromone_map, prev_pos[current_ant_idx])
                
                if hole_detection(ants, current_ant_idx, next_step, environment):
                    ant.position_x = next_step.x
                    ant.position_y = next_step.y
                    ant.state = STILL
                else:
                    ant.position_x = next_step.x
                    ant.position_y = next_step.y

        elif ant.state == STILL:
            if over_weight_detection(ants, ant):
                ant.state = WARNING
            elif no_weight_detection(ants, ant) and not bridge_detection(ants, current_ant_idx):
                ant.state = MOVING
        
        elif ant.state == WARNING:
            if not over_weight_detection(ants, ant):
                ant.state = STILL

        moving_presence = False
        for j in range(NUM_DRONE_ANT, NUM_ANT):
            worker_ant = ants[j]
            if worker_ant.state == MOVING and (worker_ant.position_x != target.x or worker_ant.position_y != target.y):
                moving_presence = True
                break

    time.sleep(2)
    print(f"{COLOR_BOLD_RED}\n\nMOST OF THE ANTS HAVE REACHED THE TARGET!{COLOR_RESET}")
    print_states(ants)

if __name__ == "__main__":
    main()