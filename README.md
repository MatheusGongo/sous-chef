> Documentação do projeto SousChef, que envolve ESP32 como Hardware principal e a Linguagem C como Linguagem de Programação principal.
> 

# Bibliotecas Utilizadas

```c
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "ESPAsyncWebServer.h"
```

---

# Variáveis/Constantes e Defines

```c
#define ONE_WIRE_BUS 4

const int relay = 2;
float relayTemMax = 102.0;
float tempMaxSousVide; 
float tempMinSousVide = 48.0;

String state = "";
   
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress sensor1 = { 0x28, 0x48, 0x31, 0x45, 0x92, 0x5, 0x2, 0xC6 };
DeviceAddress sensor2 = { 0x28, 0x6F, 0xAD, 0x45, 0x92, 0x10, 0x2, 0x70 };

const char* ssid = "NOME-DA-REDE";
const char* password = "SENHA";
const char* serverName = "NOME-DO-SERVIDOR";
const char* getRecipes = "ROTA-DE-API-GET";

String apiKeyValue = "CHAVE-API";
String sensorName = "NOME-DO-SENSOR";
String sensorLocation = "LOCAL-DO-AMBIENTE-DO-SENSOR";

AsyncWebServer server(80);
```

---

# Rotas do WebServer

```c
server.on("/temperature_water", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/plain", readTemperatureWater().c_str());
});

// Rota das receitas / web page
server.on("/receitas", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", receitas_html);
});

// Rota da receitas-detalhes/ web page
server.on("/1", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", peixe_receita_html);
});
server.on("/2", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", carne_receita_html);
});
server.on("/3", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", brocolis_receita_html);
});

server.on("/conectar/", HTTP_GET, [](AsyncWebServerRequest *request){
  AsyncWebParameter* p = request->getParam(0);
  AsyncWebParameter* temp = request->getParam(1);
  
  state = p->value();
  tempMaxSousVide = temp->value().toFloat();
  
  request->send_P(200, "text/html", prepare_html, processor);
});

server.on("/finalizado", HTTP_GET, [](AsyncWebServerRequest *request){
  AsyncWebParameter* p = request->getParam(0);
  
  state = "finalizado";
  
  request->send_P(200, "text/html", finalizado_html, processor);
});

// Rota para o gráfico / web page
server.on("/graph", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", index_html, processor);
});
```

---

# **Métodos**

### readTemperatureWater()

### **Descrição**

<aside>
📖 Método responsável por retornar a temperatura do sensor DS18b20 que está na água.

</aside>

### Sintaxe

readTemperatureWater().

### Parâmetros

```c
Nenhum.
```

### Retorna

String

### **Código de exemplo**

```c
server.on("/temperature_water", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/plain", readTemperatureWater()
});
```

---

### readTemperatureRelay()

### **Descrição**

<aside>
📖 Método responsável por retornar a temperatura do sensor DS18b20 que está no compartimento do Relê.

</aside>

### Sintaxe

readTemperatureRelay().

### Parâmetros

```c
Nenhum.
```

### Retorna

Nada

### **Código de exemplo**

```c
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURA1"){
    return readTemperatureWater();
  }
  else if(var == "TEMPERATURA2"){
    return readTemperatureRelay();
  }
  return String();
}
```

---

# Macros

```c
const char receitas_html[] PROGMEM = R"rawliteral()
```

### **Descrição**

<aside>
📖 Macro responsável por renderizar conteúdo HTML e JavaScript da página de Receitas.

</aside>

### Sintaxe

receitas_html[].

### Parâmetros

```c
Nenhum.
```

### Retorna

rawliteral

### **Código de exemplo**

```c
const char receitas_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <meta charset="utf-8" />
        <script src="https://cdn.tailwindcss.com"></script>
        <script src ="https://cdnjs.cloudflare.com/ajax/libs/axios/1.2.3/axios.min.js"></script>
    </head>
    <body class="flex items-center justify-center">
        <div
            class="xl:w-3/4 2xl:w-4/5 w-full bg-white md:p-10 p-4 rounded-lg shadow">
            <div class="text-gray-800 text-3xl font-bold mb-8 mt-2">Receitas</div>
            <div id="data-recipes" class="flex flex-wrap w-full"></div>
        </div>
        
        <script>
            async function getData() {
                let response = await
                axios.get("http://souschefapp.000webhostapp.com/get-recipes-data.php");

                let cardList = document.getElementById('data-recipes')

                let recipes = response.data;
                

                recipes.forEach(function(recipe) {
                    let convertTemp = recipe.temp_preparo / 60;

                    var cardContent = document.createElement('div');

                    cardContent.innerHTML = 
                `           <a class="w-full" href="/${recipe.idReceita}">
                                <div
                                    class="p-6 bg-white rounded-xl border-2 border-gray-200 mb-4">
                                    <img class="w-full object-cover rounded-t-md"
                                        src="${recipe.imgReceita}" alt />
                                    <div class="mt-4">
                                        <h1 class="text-2xl font-bold text-gray-700">${recipe.titulo}</h1>
                                        <div
                                            class="flex justify-start text-gray-700 items-center mt-4">
                                            <div>
                                                <svg xmlns="http://www.w3.org/2000/svg"
                                                    fill="none" viewBox="0 0 24 24"
                                                    stroke-width="1.5" stroke="currentColor"
                                                    class="w-6 h-6">
                                                    <path stroke-linecap="round"
                                                        stroke-linejoin="round"
                                                        d="M15.362 5.214A8.252 8.252 0 0112 21 8.25 8.25 0 016.038 7.048 8.287 8.287 0 009 9.6a8.983 8.983 0 013.361-6.867 8.21 8.21 0 003 2.48z" />
                                                    <path stroke-linecap="round"
                                                        stroke-linejoin="round"
                                                        d="M12 18a3.75 3.75 0 00.495-7.467 5.99 5.99 0 00-1.925 3.546 5.974 5.974 0 01-2.133-1A3.75 3.75 0 0012 18z" />
                                                </svg>
                                            </div>
                                            <div class="text-sm">Temperatura: ${recipe.temperatura}°C</div>
                                        </div>

                                        <div
                                            class="flex justify-start text-gray-700 items-center mt-4">
                                            <div>
                                                <svg xmlns="http://www.w3.org/2000/svg"
                                                    fill="none" viewBox="0 0 24 24"
                                                    stroke-width="1.5" stroke="currentColor"
                                                    class="w-6 h-6">
                                                    <path stroke-linecap="round"
                                                        stroke-linejoin="round"
                                                        d="M12 6v6h4.5m4.5 0a9 9 0 11-18 0 9 9 0 0118 0z" />
                                                </svg>
                                            </div>
                                            <div class="text-sm">Tempo de Preparo: ${convertTemp} min</div>
                                        </div>
                                    </div>
                                </div>
                            </a>
                
                `

                cardList.appendChild(cardContent);

                })    
            }
            getData();            
        </script>
    </body>
</html>)rawliteral";
```

---

```c
const char grafico_html[] PROGMEM = R"rawliteral()
```

### **Descrição**

<aside>
📖 Macro responsável por renderizar conteúdo HTML e JavaScript da página do Gráfico.

</aside>

### Sintaxe

grafico_html[].

### Parâmetros

```c
Nenhum.
```

### Retorna

rawliteral

### **Código de exemplo**

```c
const char grafico_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="utf-8"/>
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <script src="https://code.highcharts.com/highcharts.js"></script>
  <script src="https://cdn.tailwindcss.com"></script>
</head>
<body class="flex items-center justify-center">
   <div class="xl:w-3/4 2xl:w-4/5 w-full bg-white md:p-10 p-4 rounded-lg shadow">
        <div class="lg:flex justify-between w-full items-center">
              <div id="chart-temperature" class="container"></div>
        </div>
 
    </div>
   
</body>
<script>

var chartT = new Highcharts.Chart({
  chart:{ renderTo : 'chart-temperature' },
  title: { text: 'Gráfico da Temperatura em Tempo Real' },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
    series: { color: '#059e8a' }
  },
  xAxis: { type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: 'Temperature (Celsius)' }
    //title: { text: 'Temperature (Fahrenheit)' }
  },
  credits: { enabled: false }
});
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var x = (new Date()).getTime(),
          y = parseFloat(this.responseText);

      if(chartT.series[0].data.length > 40) {
        chartT.series[0].addPoint([x, y], true, true, true);
      } else {
        chartT.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/temperature_water", true);
  xhttp.send();
}, 5000 ) ;
</script>
</html>)rawliteral";
```
