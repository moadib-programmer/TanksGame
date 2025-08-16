#ifndef INFOPAGE_H
#define INFOPAGE_H

String htmlHeader = R"(
  <!DOCTYPE html>
  <html lang="en">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Information Page</title>
      <style>
  body {
    font-family: Arial, sans-serif;
    background: linear-gradient(135deg, #141E30, #243B55);
    color: #EAEAEA;
    display: flex;
    justify-content: center;
    padding: 20px;
  }

  .container {
    background: rgba(20, 30, 48, 0.9);
    border-radius: 12px;
    padding: 20px;
    width: 90%;
    max-width: 800px;
    box-shadow: 0 0 15px rgba(0, 200, 255, 0.3);
  }

  h2 {
    text-align: center;
    color: #00C8FF;
    margin-bottom: 20px;
  }

  form {
    margin-bottom: 20px;
    padding: 15px;
    border: 1px solid rgba(0, 200, 255, 0.3);
    border-radius: 10px;
    background: rgba(36, 59, 85, 0.6);
  }

  label {
    display: block;
    margin-bottom: 5px;
    color: #E0F7FF;
    font-weight: bold;
  }

  input[type="text"], select {
    width: 100%;
    padding: 8px;
    margin-bottom: 12px;
    border-radius: 8px;
    border: 1px solid #00C8FF;
    background: #1B2A41;
    color: #FFF;
    outline: none;
  }

  input[type="text"]:focus, select:focus {
    border-color: #66E0FF;
    box-shadow: 0 0 10px #00C8FF;
  }

  button {
    width: 100%;
    padding: 10px;
    border-radius: 8px;
    border: none;
    background: #00C8FF;
    color: #000;
    font-weight: bold;
    cursor: pointer;
  }

  button:hover {
    background: #66E0FF;
  }
    </style>
</head>
<body>
    <div class="container">
        <h1>Information Page</h1>
        <form action="/tankData" method="post">
)";

String htmlFooter = R"(
            <button type="submit" class="submit-button">Submit</button>
        </form>
    </div>
</body>
</html>
)";


String teamFormStart = R"(
            <div class="form-block">
                <h2>Team )";
String teamFormMid = R"(</h2>
                <input type="text" name="team)";
String teamFormEnd = R"(Name" placeholder="Team )";
String teamFormEnd2 = R"(" required>
)";

String playerFormStart = R"(
                <input type="text" name="player)";
String playerFormMid = R"(" placeholder="Player )";
String playerFormEnd = R"(" required>
)";

String targetInputStart = R"(
                <div class="target-input">
                    <label>Target )";
String targetInputMid = R"(</label>
                    <input type="number" name="target)";
String targetInputSoft = R"(SoftScore" placeholder="Soft Hit Score" required>
                    <input type="number" name="target)";
String targetInputHard = R"(HardScore" placeholder="Hard Hit Score" required>
                </div>
)";
String targetInputEnd = R"( Score to be minus" required>
)";

#endif