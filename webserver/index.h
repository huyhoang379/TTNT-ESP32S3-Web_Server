/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-web-server
 */

#ifndef WEBPAGE_H
#define WEBPAGE_H

const char* webpage = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IoT Dashboard</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: auto;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: auto;
            background-color: #f0f0f0;
        }

        .container {
            width: 90%;
            max-width: 1200px;
            background-color: #fff;
            padding: 20px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            border-radius: 10px;
        }

        h1, h2 {
            color: #333;
            text-align: center;
            margin-bottom: 1%;
        }

        .section {
            margin-bottom: 20px;
            border: 1px solid #ddd;
            padding: 20px;
            border-radius: 10px;
        }

        .section h2 {
            margin-top: 0;
        }

        .sensors {
            display: grid;
            gap: 20px;
        }

        .sensor, .device {
            border: 1px solid #ddd;
            border-radius: 5px;
            padding: 15px;
            background-color: #f9f9f9;
            
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
        }

        .sensor:nth-child(1) { background-color: #ff9999; }
        .sensor:nth-child(2) { background-color: #99ccff; }
        .sensor:nth-child(3) { background-color: #99ff99; }
        .sensor:nth-child(4) { background-color: #ffff99; }

        .sensor p {
            font-size: 1.2em;
            color: #555;
            text-align: center;
            margin-top: 2%;
            margin-bottom: auto;
        }

        .device p {
            font-size: 1.2em;
            color: #d9534f;
            text-align: center;
            margin-top: auto;
            margin-bottom: 5%;
        }

        .device h3 {
            margin-top: auto;
            margin-bottom: 5%;
        }

        .device-controls {
            display: flex;
            gap: 10px;
            justify-content: center;
            flex-wrap: wrap;
        }

        .control-port {
            display: flex;
            align-items: center;
            justify-content: center;
            flex-direction: column;
        }

        .control-port label, .control-port select, .control-port button {
            margin: 5px;
        }

        select {
            padding: 10px;
            font-size: 1em;
            border: 1px solid #ddd;
            border-radius: 5px;
            text-align: center;
        }

        button {
            padding: 10px 20px;
            font-size: 1em;
            margin: 5px;
            border: none;
            border-radius: 5px;
            background-color: #007bff;
            color: #fff;
            cursor: pointer;
            transition: background-color 0.3s;
        }

        button:hover {
            background-color: #0056b3;
        }

        /* Responsive grid for large screens */
        @media (min-width: 600px) {
            .sensors {
                grid-template-columns: repeat(2, 1fr);
            }
        }

        /* Responsive grid for large screens */
        @media (min-width: 1024px) {
            .sensors {
                grid-template-columns: repeat(4, 1fr);
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>IoT Dashboard</h1>

        <!-- Sensor Data Section -->
        <div class="section">
            <h2>Dữ liệu cảm biến</h2>
            <div class="sensors">
                <div class="sensor">
                    <h2>Nhiệt độ</h2>
                    <p id="temperature">Loading...</p>
                </div>

                <div class="sensor">
                    <h2>Độ ẩm</h2>
                    <p id="humidity">Loading...</p>
                </div>

                <div class="sensor">
                    <h2>Độ sáng</h2>
                    <p id="light">Loading...</p>
                </div>

                <div class="sensor">
                    <h2>Độ ẩm đất</h2>
                    <p id="soilMoisture">Loading...</p>
                </div>
            </div>
        </div>

        <!-- Device Control Section -->
        <div class="section">
            <h2>Điều khiển Thiết bị</h2>
            <div class="device-controls">
                <div class="device">
                    <h3>Quạt</h3>
                    <p id="fan-status">Off</p>
                    <button onclick="toggleDevice('fan')">Toggle Quạt</button>
                </div>
                <div class="device">
                    <h3>Đèn</h3>
                    <p id="light-status">Off</p>
                    <button onclick="toggleDevice('light')">Toggle Đèn</button>
                </div>
                <div class="device">
                    <h3>Máy bơm</h3>
                    <p id="pump-status">Off</p>
                    <button onclick="toggleDevice('pump')">Toggle Máy bơm</button>
                </div>
            </div>
        </div>

        <!-- Modbus RTU Control Section -->
        <div class="section">
            <h2>Điều khiển cổng Modbus RTU 485</h2>
            <div class="control-port">
                <label for="port-number">Chọn số cổng:</label>
                <select id="port-number">
                    <!-- Tạo các tùy chọn cho danh sách chọn -->
                    <script>
                        for (let i = 1; i <= 32; i++) {
                            document.write('<option value="' + i + '">Cổng ' + i + '</option>');
                        }
                    </script>
                </select>
                <button id="control-button" onclick="togglePort()">Cổng 1: Off</button>
            </div>
        </div>
    </div>

    <script>
        let portStates = Array(32).fill(false); // Mảng lưu trữ trạng thái của 32 cổng
        let deviceStates = {
            fan: false,
            light: false,
            pump: false
        };

        // Function to toggle a device
        function toggleDevice(device) {
            const statusElement = document.getElementById(`${device}-status`);
            deviceStates[device] = !deviceStates[device];

            const state = deviceStates[device] ? 'on' : 'off';
            fetch(`/?device=${device}&state=${state}`)
                .then(response => response.text())
                .then(data => {
                    if (deviceStates[device]) {
                        statusElement.textContent = 'On';
                        statusElement.style.color = '#28a745'; // Màu xanh cho On
                    } else {
                        statusElement.textContent = 'Off';
                        statusElement.style.color = '#d9534f'; // Màu đỏ cho Off
                    }
                });
        }

        // Function to toggle the port
        function togglePort() {
            const portNumber = document.getElementById('port-number').value;
            const button = document.getElementById('control-button');
            const portIndex = portNumber - 1;

            portStates[portIndex] = !portStates[portIndex];

            const state = portStates[portIndex] ? 'on' : 'off';
            fetch(`/?port=${portNumber}&state=${state}`)
                .then(response => response.text())
                .then(data => {
                    if (portStates[portIndex]) {
                        button.textContent = `Cổng ${portNumber}: On`;
                        button.style.backgroundColor = '#28a745'; // Màu xanh cho On
                    } else {
                        button.textContent = `Cổng ${portNumber}: Off`;
                        button.style.backgroundColor = '#d9534f'; // Màu đỏ cho Off
                    }
                });
        }

        // Update button text and color when changing the port number
        document.getElementById('port-number').addEventListener('change', function() {
            const portNumber = this.value;
            const button = document.getElementById('control-button');
            const portIndex = portNumber - 1;

            if (portStates[portIndex]) {
                button.textContent = `Cổng ${portNumber}: On`;
                button.style.backgroundColor = '#28a745'; // Màu xanh cho On
            } else {
                button.textContent = `Cổng ${portNumber}: Off`;
                button.style.backgroundColor = '#d9534f'; // Màu đỏ cho Off
            }
        });

        // Fetching sensor data (this should be replaced with actual data fetching logic)
        function fetchSensorData() {
            // Replace with actual fetch calls
            // document.getElementById('temperature').textContent = '25°C';
            // document.getElementById('humidity').textContent = '60%';
            // document.getElementById('light').textContent = '300';
            // document.getElementById('soilMoisture').textContent = '20';
            fetch("/temperature")
                .then(response => response.text())
                .then(data => {
                    document.getElementById("temperature").textContent = data + '°C';
                });
            fetch("/humidity")
                .then(response => response.text())
                .then(data => {
                    document.getElementById("humidity").textContent = data + '%';
                });
            fetch("/soilMoisture")
                .then(response => response.text())
                .then(data => {
                    document.getElementById("soilMoisture").textContent = data;
                });
            fetch("/light")
                .then(response => response.text())
                .then(data => {
                    document.getElementById("light").textContent = data;
                });
        }

        // Call fetchSensorData periodically (every 3 seconds)
        setInterval(fetchSensorData, 3000);
    </script>
</body>
</html>
)=====";

#endif
