const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const app = express();
const PORT = process.env.PORT || 3000;

// Middleware to parse JSON bodies and enable CORS
app.use(bodyParser.json());
app.use(cors());

// This object will store the robot data
let robotData = {
  location: null,
  batteryLevel: null,
  trashLevel: null,
  wifiLocation: null
};

// Route to receive updates from Raspberry Pi
app.post('/update', (req, res) => {
  const { location, batteryLevel, trashLevel, wifiLocation } = req.body;
  robotData = { location, batteryLevel, trashLevel, wifiLocation };
  console.log('Updated robot data:', robotData);
  res.send('Data updated successfully');
});

// Route to send data to the Flutter app
app.get('/data', (req, res) => {
  res.json(robotData);
});

// Starting the server
app.listen(PORT, () => {
  console.log(`Server listening on port ${PORT}`);
});
