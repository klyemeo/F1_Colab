<!DOCTYPE html>
<html lang="th">
<head>
  <meta charset="UTF-8">
  <title>สถานะระบบแบตเตอรี่ ESP32</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Sarabun&display=swap" rel="stylesheet">
  <style>
    body {
      font-family: 'Sarabun', sans-serif;
      background: linear-gradient(to right, #f0f8ff, #ffffff);
      color: #333;
      padding: 30px;
      text-align: center;
    }
    h1 {
      color: #0077cc;
    }
    .container {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 20px;
      margin-top: 30px;
    }
    .card {
      background: white;
      padding: 20px;
      width: 280px;
      border-radius: 10px;
      box-shadow: 0 0 10px rgba(0,0,0,0.1);
    }
    .label {
      color: #888;
      font-size: 1rem;
    }
    .value {
      font-size: 2rem;
      font-weight: bold;
      margin: 10px 0;
      color: #0077cc;
    }
    footer {
      margin-top: 40px;
      font-size: 0.9rem;
      color: #777;
    }
  </style>
</head>
<body>

  <h1>สถานะระบบแบตเตอรี่ (ESP32 + Firebase)</h1>

  <div class="container">
    <div class="card"><div class="label">กระแส (A)</div><div class="value" id="current">--</div></div>
    <div class="card"><div class="label">แรงดัน (V)</div><div class="value" id="voltage">--</div></div>
    <div class="card"><div class="label">แบตคงเหลือ (%)</div><div class="value" id="soc">--</div></div>
    <div class="card"><div class="label">Cell สูงสุด (V)</div><div class="value" id="cellHigh">--</div></div>
    <div class="card"><div class="label">Cell ต่ำสุด (V)</div><div class="value" id="cellLow">--</div></div>
    <div class="card"><div class="label">Temp สูงสุด (°C)</div><div class="value" id="tempHigh">--</div></div>
    <div class="card"><div class="label">Temp ต่ำสุด (°C)</div><div class="value" id="tempLow">--</div></div>
    <div class="card"><div class="label">กำลังชาร์จ</div><div class="value" id="charging">--</div></div>
    <div class="card"><div class="label">Fault Code</div><div class="value" id="fault">--</div></div>
    <div class="card"><div class="label">Warning Code</div><div class="value" id="warning">--</div></div>
    <div class="card"><div class="label">Timestamp</div><div class="value" id="timestamp">--</div></div>
  </div>

  <footer>
    ข้อมูลอัปเดตทุก 1 วินาที | โดย ESP32 ผ่าน Firebase
  </footer>

  <script>
    const url = "https://test-data-f19-default-rtdb.asia-southeast1.firebasedatabase.app/ESP32_Data/Location1.json";

    async function fetchData() {
      try {
        const res = await fetch(url);
        const data = await res.json();

        document.getElementById("current").textContent = data.Current?.toFixed(2) ?? "--";
        document.getElementById("voltage").textContent = data.Voltage?.toFixed(2) ?? "--";
        document.getElementById("soc").textContent = data.SOC_Percent?.toFixed(1) ?? "--";
        document.getElementById("cellHigh").textContent = data.Cell_Highest_V?.toFixed(3) ?? "--";
        document.getElementById("cellLow").textContent = data.Cell_Lowest_V?.toFixed(3) ?? "--";
        document.getElementById("tempHigh").textContent = data.Temp_High ?? "--";
        document.getElementById("tempLow").textContent = data.Temp_Low ?? "--";
        document.getElementById("charging").textContent = data.Charging ? "🔌 กำลังชาร์จ" : "⚡ ไม่ได้ชาร์จ";
        document.getElementById("fault").textContent = data.Fault_Code ?? "--";
        document.getElementById("warning").textContent = data.Warning_Code ?? "--";
        document.getElementById("timestamp").textContent = data.Timestamp ?? "--";

      } catch (err) {
        console.error("โหลดข้อมูลไม่สำเร็จ:", err);
      }
    }

    setInterval(fetchData, 1000);
    fetchData();
  </script>

</body>
</html>
