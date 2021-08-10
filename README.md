<h1>Ludo Game Using Multi-threading in C++</h1>
<p>In this i have build a ludo game using mutithreading.
  Ludo is a strategy board game for two to four players, in which the players race their
four tokens from start to finish according to the rolls of a single dies. This project
consists of designing a multithreaded application of Ludo for four players each having
maximum four tokens. Two, three or four can play, without partnerships. At the
beginning of the game, each player’s tokens are out of play and staged in the player's
yard (one of the large corner areas of the board in the player's color). When able to, the
players will enter their tokens one per time on their respective starting squares, and
proceed to race them clockwise around the board along the game track (the path of
squares not part of any player's home column). When reaching the square below his
home column, a player continues by moving tokens up the column to the finishing
square.</p>

<strong>Here is the strategy used to implement mutithreading in the Game</strong>
<ul>
  <li>Four threads are created and will be assigned to each player.</li>
  <li> A thread to check each row and column of the grid to find out the token and player to be passed for completion. This thread should run randomly to check the hitting              token and victim player.</li>
  <li> One thread known as “Master thread” should track for all four players of hit record asno token can enter into its home column if its hit rate is 0. The property of this          thread is it can cancel all the other four threads in certain situations. For example: The stopping criteria or thread cancelation will be as follows: </li>
  <li>When all tokens of a player reach its home, it will send the signal to this master thread about its completion. The master thread will verify and then kill the
      respective thread after calculating its position.</li>  
</ul>
    
<h2>Dependencies</h2>
<ul>
  <li><strong>linux Operating System</strong></li>
  <li><strong>Pthread library</strong></li>
  <li><strong>Open Gl Graphics</strong></li>
</ul>
    
<h2>Requirements to Compiled</h2>
<ul>
  <li><strong>Operating System</strong>:- Any Linux Operating System is required since pthread is not supported in windows OS.</li>
  <li><strong>Compiler</strong>:- G++ compiler is required to compile the code</li>
  <li><strong>Graphic</strong>:-Open Gl graphic is required for game graphics</li>
</ul>
  
<h2>How to Run</h2>
<ul>
  <li>Run this command to install necessary graphics of the game<em> " sudo install install-libraries.sh"</em></li>
  <li>Run command make to compile the game <em>"make"</em></li>
  <li>Run command ./game to play <em>"./game"</em></li>
  <li>You are done Have fun playing the game&#128578;&#128578;&#128578;&#128578;</li>
</ul>
<em>Contributions are wellcome if you notice any bug and wanted to implement any specific additional functionality.</em>
