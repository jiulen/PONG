# PONG?
PONG? is a physics based game similar to badminton. It includes various physics features like collision detection and response, gravity, friction, and rolling.

Controls:
(Player 1 - Left Side, Red Colour)
A to move left
D to move right
W to jump
(Player 2 - Right Side, Cyan Colour)
Left Arrow to move left
Right Arrow to move right
Up Arrow to jump

Objective: 
First to reach 10 points wins
If tied at 9-9, enter sudden death, spawns 5 balls on each side after countdown, reduce gravity to -40
once one or more balls go out the game ends, player with more points win, if same, tie.

Get 1 point by hitting ball past bottom part of opposing player's side
Also if a player hits the ball out of bounds(outside left or right boundary), other player get 1 point

When player touch ball, ball change to player colour, shows who last hit the ball

Physics:
Gravity
Inelastic Collision
Ball-ball collision, ball-wall collision and ball-pillar collision
Friction
Ball Spinning caused by friction
Rolling (only on GO_WALL)
