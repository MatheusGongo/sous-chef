#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "ESPAsyncWebServer.h"
 
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


const char* ssid = "2G-JLEAL108";  // Enter SSID here
const char* password = "123Buzu280492";  //Enter Password here
const char* serverName = "http://souschefapp.000webhostapp.com/post-esp-data.php";
const char* getRecipes = "http://souschefapp.000webhostapp.com/get-recipes-data.php";

String apiKeyValue = "tPmAT5Ab3j7F9";
String sensorName = "DS18B20";
String sensorLocation = "SousChef";

AsyncWebServer server(80);

String readTemperatureWater() {
  
  sensors.requestTemperatures();

  float sensorTempWater = sensors.getTempC(sensor1); 
  if (isnan(sensorTempWater)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(sensorTempWater);
    return String(sensorTempWater);
  }
}

String readTemperatureRelay() {

  sensors.requestTemperatures();

  float sensorTempRelay = sensors.getTempC(sensor2); 
  if (isnan(sensorTempRelay)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(sensorTempRelay);
    return String(sensorTempRelay);
  }
}

// HTML Index
const char index_html[] PROGMEM = R"rawliteral(
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

// HTML Index FIM

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



const char peixe_receita_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="utf-8"/>
  <script src="https://cdn.tailwindcss.com"></script>
</head>
<body class="flex items-center justify-center">
   <div class="xl:w-3/4 2xl:w-4/5 w-full bg-white md:p-10 p-4 rounded-lg shadow">
        <div class="flex items-center mb-8 mt-2">
          <div class="text-gray-800 mr-2">
            <a href="/receitas">
              <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" class="w-6 h-6">
                <path stroke-linecap="round" stroke-linejoin="round" d="M10.5 19.5L3 12m0 0l7.5-7.5M3 12h18" />
              </svg>
            </a>
          </div>
          <div class="text-gray-800 text-3xl font-bold">Detalhes</div>
        </div>
        <img class="w-full object-cover rounded-t-md" src="https://i.ibb.co/P1H7Kty/dc783b20-0a4b-4ff0-92cb-3d1843e48413.jpg" alt="" />
        
        <div class="text-gray-800 text-xl font-bold mb-2 mt-8">Ingredientes:</div>
        <p class="text-gray-700 font-normal text-sm">60 gramas de Cogumelo Paris fatiado</p>
        <p class="text-gray-700 font-normal text-sm">20 gramas de Cenoura ralada</p>
        <p class="text-gray-700 font-normal text-sm">20 gramas de Vagem fatiada</p>
        <p class="text-gray-700 font-normal text-sm">20 gramas de Pimentão Vermelho fatiado</p>
        <p class="text-gray-700 font-normal text-sm">1 Xícara de Molho Shoyou</p>
        <p class="text-gray-700 font-normal text-sm">Alho e Sal a gosto</p>
        <p class="text-gray-700 font-normal text-sm">Azeite a gosto</p>


        <div class="text-gray-800 text-xl font-bold mb-2 mt-8">Preparo:</div>
        <p class="text-gray-700 font-normal text-sm">Utilize sal e alho a gosto para temperar o seu cogumelo junto com os legumes. 
        Coloque as na embalagem. Com os legumes na embalagem, coloque uma xícara de molho Shoyou. 
        Mexa bem a embalagem para espalhar os temperos e embale a vácuo. 
        Utilize o nosso SousChef profissional para realizar a cocção a vácuo (sous vide). Passado o tempo, retire com cuidado do SousChef e da embalagem. 
        Bom apetite</p>

        <div class="flex justify-center mt-8 space-x-4">

                <div
                    class="w-[100.12px] h-[129.23px] p-[0px] flex-col justify-center items-center inline-flex">
                    <div class="w-[100.04px] h-[129.17px] relative">
                        <div
                            class="w-[100.04px] h-[129.17px] left-[0px] top-[0px] absolute bg-white rounded-md border border border border border-gray-200"></div>
                        <div
                            class="w-[54.98px] h-[21.11px] left-[23px] top-[89.98px] absolute text-center text-neutral-600 text-[16px] font-semibold tracking-wide">18
                            min</div>
                        <div
                            class="w-[53.59px] h-[53.59px] left-[22.63px] top-[9.51px] absolute">
                            <div
                                class="w-[53.59px] h-[53.59px] left-[0px] top-[0px] absolute bg-rose-100 rounded-full text-pink-900 flex items-center justify-center">
                            
                                <svg xmlns="http://www.w3.org/2000/svg"
                                fill="none" viewBox="0 0 24 24"
                                stroke-width="1.5" stroke="currentColor"
                                class="w-10 h-10">
                                <path stroke-linecap="round"
                                    stroke-linejoin="round"
                                    d="M12 6v6h4.5m4.5 0a9 9 0 11-18 0 9 9 0 0118 0z" />
                            </svg>
                            
                            </div>
                            <div
                                class="w-[37px] h-[37px] left-[8.37px] top-[8.42px] absolute"></div>
                        </div>
                        <div
                            class="w-[73.12px] h-[12.56px] left-[13px] top-[71.98px] absolute text-center text-neutral-600 text-[10px] font-medium tracking-wide">Tempo</div>
                    </div>
                </div>

                <div
                    class="w-[100.12px] h-[129.23px] p-[0px] flex-col justify-center items-center inline-flex">
                    <div class="w-[100.04px] h-[129.17px] relative">
                        <div
                            class="w-[100.04px] h-[129.17px] left-[0px] top-[0px] absolute bg-white rounded-md border border border border border-gray-200"></div>
                        <div
                            class="w-[33.77px] h-[21.11px] left-[33px] top-[89.98px] absolute text-center text-neutral-600 text-[16px] font-semibold tracking-wide">82°</div>
                        <div
                            class="w-[53.59px] h-[53.59px] left-[22.63px] top-[9.51px] absolute bg-rose-100 rounded-full text-pink-900 flex items-center justify-center">
                                <svg xmlns="http://www.w3.org/2000/svg"
                                    fill="none" viewBox="0 0 24 24"
                                    stroke-width="1.5" stroke="currentColor"
                                    class="w-10 h-10">
                                    <path stroke-linecap="round"
                                        stroke-linejoin="round"
                                        d="M15.362 5.214A8.252 8.252 0 0112 21 8.25 8.25 0 016.038 7.048 8.287 8.287 0 009 9.6a8.983 8.983 0 013.361-6.867 8.21 8.21 0 003 2.48z" />
                                    <path stroke-linecap="round"
                                        stroke-linejoin="round"
                                        d="M12 18a3.75 3.75 0 00.495-7.467 5.99 5.99 0 00-1.925 3.546 5.974 5.974 0 01-2.133-1A3.75 3.75 0 0012 18z" />
                                </svg>
                        
                        </div>
                        <div
                            class="w-[73.12px] h-[12.56px] left-[13px] top-[71.98px] absolute text-center text-neutral-600 text-[10px] font-medium tracking-wide">Temperatura</div>
                        <div
                            class="w-[45px] h-[45px] left-[27px] top-[11.94px] absolute"></div>
                    </div>
                </div>

            </div>

        <a href="/conectar/?param=ligar&temperature=82&tempo=1080" class="w-full block text-base font-semibold text-white bg-red-600 border border-red-600 rounded-md text-center p-4 mt-8">Conectar!</a>
    </div>          
</body>
</html>)rawliteral";


const char carne_receita_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <meta charset="utf-8" />
        <script src="https://cdn.tailwindcss.com"></script>
    </head>
    <body class="flex items-center justify-center">
        <div
            class="xl:w-3/4 2xl:w-4/5 w-full bg-white md:p-10 p-4 rounded-lg shadow">
            <div class="flex items-center mb-8 mt-2">
                <div class="text-gray-800 mr-2">
                    <a href="/receitas">
                        <svg xmlns="http://www.w3.org/2000/svg" fill="none"
                            viewBox="0 0 24 24" stroke-width="1.5"
                            stroke="currentColor" class="w-6 h-6">
                            <path stroke-linecap="round" stroke-linejoin="round"
                                d="M10.5 19.5L3 12m0 0l7.5-7.5M3 12h18" />
                        </svg>
                    </a>
                </div>
                <div class="text-gray-800 text-3xl font-bold">Detalhes</div>
            </div>
            <img class="w-full object-cover rounded-t-md"
                src="https://i.ibb.co/ww6XbNW/2-Iing-calabresa.jpg"
                alt />

            <div class>
                <div class="text-gray-800 text-xl font-bold mb-2 mt-8">Ingredientes:</div>
                <p class="text-gray-700 font-normal text-sm">60 gramas de
                    Calabresa defumada em tiras</p>
                <p class="text-gray-700 font-normal text-sm">Azeite extra virgem
                    (Opcional)</p>

            </div>

            <div class="text-gray-800 text-xl font-bold mb-2 mt-8">Preparo:</div>
            <p class="text-gray-700 font-normal text-sm">Coloque as tiras na
                embalagem. Com a calabresa na embalagem, coloque 02 colheres de
                azeite extra virgem (opcional). Mexa bem a embalagem para
                espalhar e embale a vácuo.
                Utilize o nosso SousChef profissional para realizar a cocção a
                vácuo (sous vide).
                Passado o tempo, retire a calabresa do SousChef e da embalagem.
                Bom apetite!</p>

            <div class="flex justify-center mt-8 space-x-4">

                <div
                    class="w-[100.12px] h-[129.23px] p-[0px] flex-col justify-center items-center inline-flex">
                    <div class="w-[100.04px] h-[129.17px] relative">
                        <div
                            class="w-[100.04px] h-[129.17px] left-[0px] top-[0px] absolute bg-white rounded-md border border border border border-gray-200"></div>
                        <div
                            class="w-[54.98px] h-[21.11px] left-[23px] top-[89.98px] absolute text-center text-neutral-600 text-[16px] font-semibold tracking-wide">25
                            min</div>
                        <div
                            class="w-[53.59px] h-[53.59px] left-[22.63px] top-[9.51px] absolute">
                            <div
                                class="w-[53.59px] h-[53.59px] left-[0px] top-[0px] absolute bg-rose-100 rounded-full text-pink-900 flex items-center justify-center">

                                <svg xmlns="http://www.w3.org/2000/svg"
                                    fill="none" viewBox="0 0 24 24"
                                    stroke-width="1.5" stroke="currentColor"
                                    class="w-10 h-10">
                                    <path stroke-linecap="round"
                                        stroke-linejoin="round"
                                        d="M12 6v6h4.5m4.5 0a9 9 0 11-18 0 9 9 0 0118 0z" />
                                </svg>

                            </div>
                            <div
                                class="w-[37px] h-[37px] left-[8.37px] top-[8.42px] absolute"></div>
                        </div>
                        <div
                            class="w-[73.12px] h-[12.56px] left-[13px] top-[71.98px] absolute text-center text-neutral-600 text-[10px] font-medium tracking-wide">Tempo</div>
                    </div>
                </div>

                <div
                    class="w-[100.12px] h-[129.23px] p-[0px] flex-col justify-center items-center inline-flex">
                    <div class="w-[100.04px] h-[129.17px] relative">
                        <div
                            class="w-[100.04px] h-[129.17px] left-[0px] top-[0px] absolute bg-white rounded-md border border border border border-gray-200"></div>
                        <div
                            class="w-[33.77px] h-[21.11px] left-[33px] top-[89.98px] absolute text-center text-neutral-600 text-[16px] font-semibold tracking-wide">78°</div>
                        <div
                            class="w-[53.59px] h-[53.59px] left-[22.63px] top-[9.51px] absolute bg-rose-100 rounded-full text-pink-900 flex items-center justify-center">
                            <svg xmlns="http://www.w3.org/2000/svg"
                                fill="none" viewBox="0 0 24 24"
                                stroke-width="1.5" stroke="currentColor"
                                class="w-10 h-10">
                                <path stroke-linecap="round"
                                    stroke-linejoin="round"
                                    d="M15.362 5.214A8.252 8.252 0 0112 21 8.25 8.25 0 016.038 7.048 8.287 8.287 0 009 9.6a8.983 8.983 0 013.361-6.867 8.21 8.21 0 003 2.48z" />
                                <path stroke-linecap="round"
                                    stroke-linejoin="round"
                                    d="M12 18a3.75 3.75 0 00.495-7.467 5.99 5.99 0 00-1.925 3.546 5.974 5.974 0 01-2.133-1A3.75 3.75 0 0012 18z" />
                            </svg>

                        </div>
                        <div
                            class="w-[73.12px] h-[12.56px] left-[13px] top-[71.98px] absolute text-center text-neutral-600 text-[10px] font-medium tracking-wide">Temperatura</div>
                        <div
                            class="w-[45px] h-[45px] left-[27px] top-[11.94px] absolute"></div>
                    </div>
                </div>

            </div>

            <a href="/conectar/?param=ligar&temperature=78&tempo=1500"
                class="w-full block text-base font-semibold text-white bg-red-600 border border-red-600 rounded-md text-center p-4 mt-8">Conectar!</a>
        </div>
    </body>
</html>)rawliteral";


const char brocolis_receita_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <meta charset="utf-8" />
        <script src="https://cdn.tailwindcss.com"></script>
    </head>
    <body class="flex items-center justify-center">
        <div
            class="xl:w-3/4 2xl:w-4/5 w-full bg-white md:p-10 p-4 rounded-lg shadow">
            <div class="flex items-center mb-8 mt-2">
                <div class="text-gray-800 mr-2">
                    <a href="/receitas">
                        <svg xmlns="http://www.w3.org/2000/svg" fill="none"
                            viewBox="0 0 24 24" stroke-width="1.5"
                            stroke="currentColor" class="w-6 h-6">
                            <path stroke-linecap="round" stroke-linejoin="round"
                                d="M10.5 19.5L3 12m0 0l7.5-7.5M3 12h18" />
                        </svg>
                    </a>
                </div>
                <div class="text-gray-800 text-3xl font-bold">Detalhes</div>
            </div>
            <img class="w-full object-cover rounded-t-md"
                src="https://i.ibb.co/KFWPPZt/3a500be3cdbb0f0d2ca5093ae08bb776.jpg"
                alt />

            <div class>
                <div class="text-gray-800 text-xl font-bold mb-2 mt-8">Ingredientes:</div>
                <p class="text-gray-700 font-normal text-sm">30 gramas de
                    brócolis</p>
                <p class="text-gray-700 font-normal text-sm">Sal e pimenta a
                    gosto</p>
                <p class="text-gray-700 font-normal text-sm">Alho (opcional)</p>
                <p class="text-gray-700 font-normal text-sm">Azeite extra virgem</p>
            </div>

            <div class>
                <div class="text-gray-800 text-xl font-bold mb-2 mt-8">Preparo:</div>
                <p class="text-gray-700 font-normal text-sm">Utilize sal e
                    pimenta a gosto para temperar o seu brócolis.
                    Coloque as peças na embalagem. Com os brócolis na embalagem,
                    coloque 03 colheres de azeite extra virgem.
                    Se você quiser, corte alho em pequenos pedaços e coloque na
                    embalagem para ajudar a temperar.
                    Mexa bem a embalagem para espalhar os temperos e embale a
                    vácuo.
                    Utilize o nosso SousChef profissional para realizar a cocção
                    a vácuo (sous vide). Siga o tempo e a temperatura que foram
                    sugeridos acima.
                    Passado o tempo, retire o brócolis do termocirculador e da
                    embalagem</p>
            </div>

            <div class="flex justify-center mt-8 space-x-4">

                <div
                    class="w-[100.12px] h-[129.23px] p-[0px] flex-col justify-center items-center inline-flex">
                    <div class="w-[100.04px] h-[129.17px] relative">
                        <div
                            class="w-[100.04px] h-[129.17px] left-[0px] top-[0px] absolute bg-white rounded-md border border border border border-gray-200"></div>
                        <div
                            class="w-[54.98px] h-[21.11px] left-[23px] top-[89.98px] absolute text-center text-neutral-600 text-[16px] font-semibold tracking-wide">15
                            min</div>
                        <div
                            class="w-[53.59px] h-[53.59px] left-[22.63px] top-[9.51px] absolute">
                            <div
                                class="w-[53.59px] h-[53.59px] left-[0px] top-[0px] absolute bg-rose-100 rounded-full text-pink-900 flex items-center justify-center">
                            
                                <svg xmlns="http://www.w3.org/2000/svg"
                                fill="none" viewBox="0 0 24 24"
                                stroke-width="1.5" stroke="currentColor"
                                class="w-10 h-10">
                                <path stroke-linecap="round"
                                    stroke-linejoin="round"
                                    d="M12 6v6h4.5m4.5 0a9 9 0 11-18 0 9 9 0 0118 0z" />
                            </svg>
                            
                            </div>
                            <div
                                class="w-[37px] h-[37px] left-[8.37px] top-[8.42px] absolute"></div>
                        </div>
                        <div
                            class="w-[73.12px] h-[12.56px] left-[13px] top-[71.98px] absolute text-center text-neutral-600 text-[10px] font-medium tracking-wide">Tempo</div>
                    </div>
                </div>

                <div
                    class="w-[100.12px] h-[129.23px] p-[0px] flex-col justify-center items-center inline-flex">
                    <div class="w-[100.04px] h-[129.17px] relative">
                        <div
                            class="w-[100.04px] h-[129.17px] left-[0px] top-[0px] absolute bg-white rounded-md border border border border border-gray-200"></div>
                        <div
                            class="w-[33.77px] h-[21.11px] left-[33px] top-[89.98px] absolute text-center text-neutral-600 text-[16px] font-semibold tracking-wide">80°</div>
                        <div
                            class="w-[53.59px] h-[53.59px] left-[22.63px] top-[9.51px] absolute bg-rose-100 rounded-full text-pink-900 flex items-center justify-center">
                                <svg xmlns="http://www.w3.org/2000/svg"
                                    fill="none" viewBox="0 0 24 24"
                                    stroke-width="1.5" stroke="currentColor"
                                    class="w-10 h-10">
                                    <path stroke-linecap="round"
                                        stroke-linejoin="round"
                                        d="M15.362 5.214A8.252 8.252 0 0112 21 8.25 8.25 0 016.038 7.048 8.287 8.287 0 009 9.6a8.983 8.983 0 013.361-6.867 8.21 8.21 0 003 2.48z" />
                                    <path stroke-linecap="round"
                                        stroke-linejoin="round"
                                        d="M12 18a3.75 3.75 0 00.495-7.467 5.99 5.99 0 00-1.925 3.546 5.974 5.974 0 01-2.133-1A3.75 3.75 0 0012 18z" />
                                </svg>
                        
                        </div>
                        <div
                            class="w-[73.12px] h-[12.56px] left-[13px] top-[71.98px] absolute text-center text-neutral-600 text-[10px] font-medium tracking-wide">Temperatura</div>
                        <div
                            class="w-[45px] h-[45px] left-[27px] top-[11.94px] absolute"></div>
                    </div>
                </div>

            </div>

            <a href="/conectar/?param=ligar&temperature=80&tempo=900"
                class="w-full block text-base font-semibold text-white bg-red-600 border border-red-600 rounded-md text-center p-4 mt-8">Conectar!</a>
        </div>
    </body>
</html>)rawliteral";



const char finalizado_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="utf-8"/>
  <script src="https://cdn.tailwindcss.com"></script>
  <script src="https://unpkg.com/@lottiefiles/lottie-player@latest/dist/lottie-player.js"></script>
</head>
<body class="flex items-center justify-center">
   <div class="xl:w-3/4 2xl:w-4/5 w-full bg-white md:p-10 p-4 rounded-lg shadow">
      <div class="flex flex-col items-center justify-center">
          <lottie-player src="https://assets4.lottiefiles.com/packages/lf20_blxqpmul.json"  background="transparent"  speed="1"  style="width: 300px; height: 300px;" loop autoplay></lottie-player>
        
          <div class="text-gray-700 font-semibold text-2xl mb-8">Preparo Finalizado!</div>
          <p class="text-gray-700 font-normal text-lg">Tenha cuidado ao abrir a sacola a vácuo. Bom apetite :D</p>
        

        <a href="/receitas" class="w-full block text-base font-semibold text-white bg-red-600 border border-red-600 rounded-md text-center p-4 mt-8">Página Inicial</a>
        </div>
    </div>         
</body>
</html>)rawliteral";


const char prepare_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="utf-8"/>
  <script src="https://cdn.tailwindcss.com"></script>
  <script src="https://unpkg.com/@lottiefiles/lottie-player@latest/dist/lottie-player.js"></script>
</head>
<body class="flex items-center justify-center">
   <div class="xl:w-3/4 2xl:w-4/5 w-full bg-white md:p-10 p-4 rounded-lg shadow">
      <div class="flex flex-col items-center justify-center">
          <lottie-player src="https://assets1.lottiefiles.com/packages/lf20_vkqybeu5/data.json"  background="transparent"  speed="1"  style="width: 300px; height: 300px;" loop autoplay></lottie-player>
        
          <div class="text-gray-700 font-semibold text-2xl mb-8">Preparando o seu Sous Vide</div>
          <div class="flex flex-col items-center w-full">
              <div
                  class="w-full h-[87px] p-[0px] justify-center items-center inline-flex">
                  <div class="w-full h-[87.12px] relative">
                      <div
                          class="w-full h-[87.12px] left-[0px] top-[0px] absolute bg-white rounded-md border border border border border-gray-200"></div>
                      <div
                          class="w-[53.59px] h-[53.59px] left-[20px] top-[15.97px] absolute">
                          <div
                              class="w-[53.59px] h-[53.59px] left-[0px] top-[0px] absolute bg-rose-100 rounded-full flex items-center justify-center text-pink-900">
                              <svg xmlns="http://www.w3.org/2000/svg"
                                  fill="none" viewBox="0 0 24 24"
                                  stroke-width="1.5" stroke="currentColor"
                                  class="w-10 h-10">
                                  <path stroke-linecap="round"
                                      stroke-linejoin="round"
                                      d="M12 6v6h4.5m4.5 0a9 9 0 11-18 0 9 9 0 0118 0z" />
                              </svg></div>
                          <div
                              class="w-[37px] h-[37px] left-[8.37px] top-[8.42px] absolute"></div>
                      </div>
                      <div
                          class="w-[165.60px] h-[53.28px] left-[82.99px] top-[17.04px] absolute">
                          <div id="time"
                              class="w-full h-8 left-[0.01px] top-[20.96px] absolute text-neutral-600 text-[24px] font-semibold tracking-wider"></div>
                          <div
                              class="w-[165.59px] h-[20.45px] left-[0.01px] top-[-0px] absolute text-neutral-600 text-[14px] font-medium tracking-wide">Tempo
                              de Cozimento</div>
                      </div>
                  </div>
              </div>
              <div class="w-full h-[87px] relative mt-6">
                  <div
                      class="w-full h-[87.12px] left-[0px] top-[0.06px] absolute">
                      <div
                          class="w-full h-[87.12px] left-[0px] top-[0px] absolute bg-white rounded-md border border border border border-gray-200"></div>
                      <div
                          class="w-[53.59px] h-[53.59px] left-[20px] top-[15.97px] absolute">
                          <div
                              class="w-[53.59px] h-[53.59px] left-[0px] top-[0px] absolute bg-rose-100 rounded-full flex items-center justify-center text-pink-900">
                              <svg xmlns="http://www.w3.org/2000/svg"
                                  fill="none" viewBox="0 0 24 24"
                                  stroke-width="1.5" stroke="currentColor"
                                  class="w-10 h-10">
                                  <path stroke-linecap="round"
                                      stroke-linejoin="round"
                                      d="M15.362 5.214A8.252 8.252 0 0112 21 8.25 8.25 0 016.038 7.048 8.287 8.287 0 009 9.6a8.983 8.983 0 013.361-6.867 8.21 8.21 0 003 2.48z" />
                                  <path stroke-linecap="round"
                                      stroke-linejoin="round"
                                      d="M12 18a3.75 3.75 0 00.495-7.467 5.99 5.99 0 00-1.925 3.546 5.974 5.974 0 01-2.133-1A3.75 3.75 0 0012 18z" />
                              </svg></div>
                          <div
                              class="w-[45px] h-[45px] left-[4px] top-[4.97px] absolute"></div>
                      </div>
                      <div
                          class="w-[165.60px] h-[53.28px] left-[82.99px] top-[17.04px] absolute">
                          <div id="temperature"
                              class="w-[104.22px] h-8 left-[0.01px] top-[20.96px] absolute text-neutral-600 text-[24px] font-semibold tracking-wider"></div>
                          <div
                              class="w-[165.59px] h-[20.45px] left-[0.01px] top-[-0px] absolute text-neutral-600 text-[14px] font-medium tracking-wide">Temperatura
                              Atual</div>
                      </div>
                  </div>
              </div>
          </div>

        <a href="/graph" class="w-full block text-base font-semibold text-red-600 bg-white border border-red-600 rounded-md text-center p-4 mt-8">Acompanhar</a>
        </div>
    </div>         
</body>
<script>

    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    
    const tempoCozimento = urlParams.get('tempo');
    const temperaturaCozimente = urlParams.get('temperature');
    console.log(tempoCozimento);
    
    function startTimer(duration, display) {
        var timer = duration, minutes, seconds;
        setInterval(function () {
            minutes = parseInt(timer / 60, 10);
            seconds = parseInt(timer % 60, 10);
    
            minutes = minutes < 10 ? "0" + minutes : minutes;
            seconds = seconds < 10 ? "0" + seconds : seconds;
    
            display.textContent = minutes + ":" + seconds + " min";
    
            if (--timer < 0) {
                timer = duration;
                window.location.href = "/finalizado";
            }
        }, 1000);
    }
    
    window.onload = function () {
        var timePrepare = tempoCozimento,
            display = document.querySelector('#time');
        startTimer(timePrepare, display);
    };

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {

            let cardTemperature = document.getElementById("temperature");
            cardTemperature.textContent = parseFloat(this.responseText) + " °C";

        }
    };
    xhttp.open("GET", "/temperature_water", true);
    xhttp.send();
    }, 1000 ) ;

</script>
</html>)rawliteral";




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

void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  sensors.begin();

  Serial.println("Connecting to ");
  Serial.println(ssid);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());



  server.on("/temperature_water", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemperatureWater().c_str());
  });

  // Route for receitas / web page
  server.on("/receitas", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", receitas_html);
  });

  // Route for receitas-detalhes/ web page
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

  // Route for root / web page
  server.on("/graph", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}


void loop() {

 sensors.requestTemperatures();

 float sensorTempWaterValidate = sensors.getTempC(sensor1); 
  if(state == "ligar"){
   if(sensorTempWaterValidate > tempMaxSousVide){
      digitalWrite(relay, LOW);
      Serial.println("ESTÁ DESLIGADO");
      Serial.println(sensorTempWaterValidate);
      delay(40000);
   } else {
      digitalWrite(relay, HIGH);
      Serial.println("ESTÁ LIGADO");

       //Check WiFi connection status
        if(WiFi.status()== WL_CONNECTED){
          
          HTTPClient http;
          
          http.begin(serverName);
          
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          
          String httpRequestData = "sensor=" + sensorName
                                + "&location=" + sensorLocation + "&temperature=" + String(sensorTempWaterValidate) + "";
          Serial.print("httpRequestData: ");
          Serial.println(httpRequestData);
          
        
           int httpResponseCode = http.POST(httpRequestData);
                
            if (httpResponseCode>0) {
              Serial.print("Temperatura: ");
              Serial.println(String(sensorTempWaterValidate));
            }
            else {
              Serial.print("Temperatura: ");
              Serial.println(String(sensorTempWaterValidate));
            } 
            // Free resources
            http.end(); 
          }
          else {
            Serial.println("WiFi Disconnected");
          }
   }
  } else if(state == "finalizado"){
      digitalWrite(relay, LOW);
      Serial.println("ESTÁ DESLIGADO");
  }




  //Send an HTTP POST request every 30 seconds
  delay(5000);  
  
}
