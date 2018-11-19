/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* ========================================================================*/
/* Navigation
/* ========================================================================*/

function selectSection(event, section) {
    
    console.log("Selected tab " +  section);

    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }
    
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    
    document.getElementById(section).style.display = "block";
    event.currentTarget.className += " active";
}


/* ========================================================================*/
/* Websocket connection
/* ========================================================================*/

var webSocket = null;
var webSocketConnected = false;

function toggleWebsocketConnection() {
    if (webSocketConnected) {
        disconnectWebsocket();
    } else {
        connectWebsocket();
    }
}


function connectWebsocket() {
    var urlString = "ws://" + window.location.hostname + ":2626";
    console.log("Connecting to: " + urlString);
    
    try {
        webSocket = new WebSocket(urlString);
        
        webSocket.onopen = function(openEvent) {
            console.log("WebSocket connected: " + JSON.stringify(openEvent, null, 4));
            webSocketConnected = true;
            document.getElementById("toggleLogsButton").innerHTML = "Stop logs";
        };
        
        webSocket.onclose = function(closeEvent) {
            console.log("WebSocket disconnected: " + JSON.stringify(closeEvent, null, 4));
            webSocketConnected = false;
            document.getElementById("toggleLogsButton").innerHTML = "Start logs";
        };
        
        webSocket.onerror = function(errorEvent) {
            console.log("WebSocket error: " + JSON.stringify(errorEvent, null, 4));
        };
        
        webSocket.onmessage = function (messageEvent) {
            var message = messageEvent.data;
            console.log("WebSocket data received: " + message);
            document.getElementById("logsTextArea").value += message;
            document.getElementById("logsTextArea").scrollTop = document.getElementById("logsTextArea").scrollHeight;
        };
    
    } catch (exception) {
        console.error(exception);
    }
    
}


function disconnectWebsocket() {
    console.log("Disconnecting from: " + webSocket.url);
    webSocket.close();
    webSocketConnected = false;
    document.getElementById("toggleLogsButton").innerHTML = "Start logs";
}


/* ========================================================================*/
/* File download / show functions
/* ========================================================================*/

function showFile(path) {
    console.log("Show file in tab " + path);
    window.open(path, '_blank');
}


function downloadFile(filePath, fileName) {
    console.log("Download file requested " + filePath + " --> " + fileName);
    var element = document.createElement('a');
    element.setAttribute('href', filePath);
    element.setAttribute('download', fileName);
    element.style.display = 'none';
    document.body.appendChild(element);
    element.click();
    document.body.removeChild(element);
}


var generateReportTimer = null;

function generateReport() {
    console.log("Requesting to generate report file " + "/debug/report");
    
    var textArea = document.getElementById("generateReportTextArea");
    var button = document.getElementById("generateReportButton");

    // Request report file generation
    var reportGenerateRequest = new XMLHttpRequest();
    reportGenerateRequest.open("GET", "/debug/report", true);
    reportGenerateRequest.send(null);
    
    button.disabled = true;
    textArea.value = ".";
    
       // Start the timer
    generateReportTimer = setTimeout(generateReportTimerTimeout, 1000);
    
    reportGenerateRequest.onreadystatechange = function() {
        if (reportGenerateRequest.readyState == 4) {
            console.log("Report generation finished with " + reportGenerateRequest.status);

            if (reportGenerateRequest.status != 200) {
                console.log("Report generation finished with error.");
                textArea.value = "Something went wrong :(";
                button.disabled = false;
                return;
            }
            
            // Stop the timer
            clearTimeout(generateReportTimer);
        
            console.log(reportGenerateRequest.responseText);
            var responseMap = JSON.parse(reportGenerateRequest.responseText);
            var fileName = responseMap['fileName'];
            var fileSize = responseMap['fileSize'];
            var md5Sum = responseMap['md5sum'];
            
            console.log("Report generation finished. " + fileName + " " + fileSize + "B | " +  md5Sum)

            textArea.value = "Report generated successfully: " + fileName + "\n";
            textArea.value += "\n";
            textArea.value += "Size: " + fileSize + " Bytes" + "\n";
            textArea.value += "MD5 checksum: " + md5Sum + "\n";
            
            // Now download the generated report
            var fileRequestUrl = "/debug/report?filename=" + fileName;
            console.log("Download report file " + fileRequestUrl);
            var element = document.createElement('a');
            element.setAttribute('href', fileRequestUrl);
            element.setAttribute('download', fileName);
            element.style.display = 'none';
            document.body.appendChild(element);
            element.click();
            document.body.removeChild(element);
            
            // Enable button again
            button.disabled = false;
        }
    };
}

function generateReportTimerTimeout() {
    var textArea = document.getElementById("generateReportTextArea");
    textArea.value += ".";
    generateReportTimer = setTimeout(generateReportTimerTimeout, 1000);
}

/* ========================================================================*/
/* Network test functions
/* ========================================================================*/

var pingTimer = null;

function startPingTest() {
    console.log("Start ping test");
    var textArea = document.getElementById("pingTextArea");
    var button = document.getElementById("pingButton");
    // Clear the text output
    textArea.value = ".";
    
    // Request ping output
    var request = new XMLHttpRequest();
    request.open("GET", "/debug/ping", true);
    request.send(null);
    
    // Start the timer
    pingTimer = setTimeout(pingTimerTimeout, 1000);
    
    button.disabled = true;
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            // Stop the timer
            clearTimeout(pingTimer);
        
            console.log(request.responseText);
            textArea.value = request.responseText;
            button.disabled = false;
        }
    };
}

function pingTimerTimeout() {
    var textArea = document.getElementById("pingTextArea");
    textArea.value += ".";
    pingTimer = setTimeout(pingTimerTimeout, 1000);
}



var digTimer = null;

function startDigTest() {
    console.log("Start dig test");
    var textArea = document.getElementById("digTextArea");
    var button = document.getElementById("digButton");
    
    // Clear the text output
    textArea.value = ".";
    
    // Request dig output
    var request = new XMLHttpRequest();
    request.open("GET", "/debug/dig", true);
    request.send(null);
    
        // Start the timer
    digTimer = setTimeout(digTimerTimeout, 1000);
    
    button.disabled = true;
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            // Stop the timer
            clearTimeout(digTimer);
        
            console.log(request.responseText);
            textArea.value = request.responseText;
            button.disabled = false;
        }
    };
}

function digTimerTimeout() {
    var textArea = document.getElementById("digTextArea");
    textArea.value += ".";
    digTimer = setTimeout(digTimerTimeout, 1000);
}



var tracePathTimer = null;

function startTracePathTest() {
    console.log("Start trace path test");
    var textArea = document.getElementById("tracePathTextArea");
    var button = document.getElementById("tracePathButton");
    
    // Clear the text output
    textArea.value = ".";
    
    // Request dig output
    var request = new XMLHttpRequest();
    request.open("GET", "/debug/tracepath", true);
    request.send(null);
    
    // Start the timer
    tracePathTimer = setTimeout(tracePathTimerTimeout, 1000);
    
    button.disabled = true;
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            // Stop the timer
            clearTimeout(tracePathTimer);
            
            console.log(request.responseText);
            textArea.value = request.responseText;
            button.disabled = false;
        }
    };
}

function tracePathTimerTimeout() {
    var textArea = document.getElementById("tracePathTextArea");
    textArea.value += ".";
    tracePathTimer = setTimeout(tracePathTimerTimeout, 1000);
}


/* ========================================================================*/
/* Start function calls
/* ========================================================================*/

window.onload = function() {
    console.log("Window loading finished.");
    document.getElementById("informationTabButton").click();
};

