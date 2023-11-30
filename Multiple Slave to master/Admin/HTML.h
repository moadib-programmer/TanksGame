
String html = R"(
  <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-A-Compatible" content="ie=edge">
    <title>Document</title>
    <style>
        .button{
          color: white; 
          background-color: green; 
          width : 70px; 
          height: 50px; 
          border: 3px solid black; 
          font-size: 20px;
        }
        p,span,h1
        {
          font-family: sans-serif;

        }
        .square 
        {
          width: 200px;
          height: 200px;
          text-align: center;
          padding: 20px;
          border-radius: 10px;
          margin: 10px;
          color: white;
          font-family: Arial, sans-serif;
        }
    </style>
</head>
)";


String dataPage = R"(
<body>
    <h1>Data Page</h1>

    <form action='/save' method='post'>
        <label style="font-size: 20px;" for="teamNum">Enter number of teams</label>
        <div>
            <input type="number" name="teamNum" required>
        </div>
        <br>

        <label style="font-size: 20px;" for="teamNames">Enter Team names (seperated by ,)</label>
        <div>
            <input type="text" name="teamNames" required>
        </div>
        <br>

        <label style="font-size: 20px;" for="tankNames">Enter Tank names (seperated by ,)</label>
        <div>
            <input type="text" name="tankNames" required>
        </div>
        <br>

        <div  id="submitButton">
            <input class="button" style="background-color:  black;" type="submit" value="Send">
        </div>
        
    </form>
</body>

</html>
)";

String startPage = R"(

<body>
    <h1>Start Game</h1>
    <p>Press the button to start the game</p>

    <form action='/start' method='post'>
        <div  id="submitButton">
            <input class="button" style="background-color:  green;" type="submit" value="Send">
        </div>
        
    </form>
</body>

</html>

)";

String scorePage = R"(
<body>
    <h1>Score Page</h1>
    <p>These are the teams and here are their scores</p>

    <div style="background-color: #3498db;" id="team1" class="square">
      <h2>Team Name: Team Alpha</h2>
      <p>Score: 85</p>
    </div>
  
    <div style="background-color: #e74c3c;" id="team2" class="square">
      <h2>Team Name: Team Bravo</h2>
      <p>Score: 72</p>
    </div>
  
    <div style="background-color: #27ae60;" id="team3" class="square">
      <h2>Team Name: Team Charlie</h2>
      <p>Score: 93</p>
    </div>
  </body>
</html>
)";