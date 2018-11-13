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
/* Websocket connection
/* ========================================================================*/

var webSocket = null;

function connectWebsocket() {
    var urlString = "ws://" + window.location.hostname + ":2626"
    console.log("Connecting to: " + urlString);
    
    try {
        webSocket = new WebSocket(urlString);
        
        webSocket.onopen = function(openEvent) {
            console.log("WebSocket connected: " + JSON.stringify(openEvent, null, 4));
            document.getElementById("connectWebsocketButton").disabled = true;
            document.getElementById("disconnectWebsocketButton").disabled = false;
        };
        
        webSocket.onclose = function (closeEvent) {
            console.log("WebSocket disconnected: " + JSON.stringify(closeEvent, null, 4));
            document.getElementById("connectWebsocketButton").disabled = false;
            document.getElementById("disconnectWebsocketButton").disabled = true;
        };
        
        webSocket.onerror = function (errorEvent) {
            console.log("WebSocket error: " + JSON.stringify(errorEvent, null, 4));
        };
        
        webSocket.onmessage = function (messageEvent) {
            var message = messageEvent.data;
            console.log("WebSocket data received: " + message);
            document.getElementById("logsTextArea").value += message;
            document.getElementById("logsTextArea").scrollTop = document.getElementById("logsTextArea").scrollHeight 
        };
    
    } catch (exception) {
        console.error(exception);
    }
    
}

function disconnectWebsocket() {
    console.log("Disconnecting from: " + webSocket.url);
    webSocket.close()
}



/* ========================================================================*/
/* File download function
/* ========================================================================*/

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

/* ========================================================================*/
/* Network test functions
/* ========================================================================*/

function startPingTest() {
    console.log("Start ping test");
    var textArea = document.getElementById("pingTextArea");
    var button = document.getElementById("pingButton");
    // Clear the text output
    textArea.value = "";
    
    // Request ping output
    var request = new XMLHttpRequest();
    request.open("GET", "/debug/ping", true);
    request.send(null);
    button.disabled = true
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            console.log(request.responseText);
            textArea.value = request.responseText
            button.disabled = false
        }
    };
}

function startDigTest() {
    console.log("Start dig test");
    var textArea = document.getElementById("digTextArea");
    var button = document.getElementById("digButton");
    
    // Clear the text output
    textArea.value = "";
    
    // Request dig output
    var request = new XMLHttpRequest();
    request.open("GET", "/debug/dig", true);
    request.send(null);
    button.disabled = true
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            console.log(request.responseText);
            textArea.value = request.responseText
            button.disabled = false
        }
    };
}

function startTracePathTest() {
    console.log("Start trace path test");
    var textArea = document.getElementById("tracePathTextArea");
    var button = document.getElementById("tracePathButton");
    
    // Clear the text output
    textArea.value = "";
    
    // Request dig output
    var request = new XMLHttpRequest();
    request.open("GET", "/debug/tracepath", true);
    request.send(null);
    button.disabled = true
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            console.log(request.responseText);
            textArea.value = request.responseText
            button.disabled = false
        }
    };
}
