/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  SPDX-License-Identifier: GPL-3.0-or-later                              *
 *                                                                         *
 *  Copyright (C) 2018 - 2024 nymea GmbH <contact@nymea.io>                *
 *  Copyright (C) 2024 - 2025 chargebyte austria GmbH <contact@nymea.io>   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <https://www.gnu.org/licenses/>.         *
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
/* Websocket connection for log live view
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
            console.log("WebSocket connected");
            webSocketConnected = true;
            document.getElementById("toggleLogsButton").innerHTML = "Stop logs";
        };
        
        webSocket.onclose = function(closeEvent) {
            console.log("WebSocket disconnected");
            webSocketConnected = false;
            document.getElementById("toggleLogsButton").innerHTML = "Start logs";
        };
        
        webSocket.onerror = function(errorEvent) {
            console.log("WebSocket error: " + JSON.stringify(errorEvent, null, 4));
        };
        
        webSocket.onmessage = function (messageEvent) {
            var message = messageEvent.data;
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


function clearLogsContent() {
    console.log("Clear live log content");
    var logTextArea = document.getElementById("logsTextArea")
    logTextArea.value = "";
}


function copyLogsContent() {
    console.log("Copy live log content");
    var logTextArea = document.getElementById("logsTextArea")
    
    logTextArea.select();
    logTextArea.setSelectionRange(0, 99999); /*For mobile devices*/
    document.execCommand("copy");
    
    console.log("Copied text:");
    console.log(logTextArea.value);
    
    /* Clear selection */
    document.select();
}


function loadLoggingCategorySettings() {
    // Request report file generation
    var request = new XMLHttpRequest();
    request.open("GET", "/debug/logging-categories", true);
    request.send(null);
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            console.log("Load logging category settings finished", request.status);

            /* Check if the generation went fine */
            if (request.status != 200) {
                console.warn("Could not load logging category settings", request.status);
                return;
            }
            
            var responseMap = JSON.parse(request.responseText);
            
            for (var loggingCategory in responseMap['loggingCategories']) {
                var loggingCategoryElement = document.getElementById("debug-category-" + loggingCategory)
                console.log("Setting category", loggingCategory, "to", responseMap['loggingCategories'][loggingCategory])
                loggingCategoryElement.value = responseMap['loggingCategories'][loggingCategory]
            }   
            
            for (var loggingCategory in responseMap['loggingCategoriesPlugins']) {
                var loggingCategoryElement = document.getElementById("debug-category-" + loggingCategory)
                loggingCategoryElement.value = responseMap['loggingCategoriesPlugins'][loggingCategory]
            }   
        }
    }
}


function toggleLoggingCategory(categoryName, obj) {
    console.log("Select changed:", categoryName, obj.value)
    
    var fileRequestUrl = "/debug/logging-categories?" + categoryName + "=" + obj.value;
    
        // Request report file generation
    var request = new XMLHttpRequest();
    request.open("GET", fileRequestUrl, true);
    request.send(null);
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            console.log("Set logging category settings finished", request.status);

            /* Check if the generation went fine */
            if (request.status != 200) {
                console.warn("Could not set logging category settings", request.status);
                return;
            }
        }
    }
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


/* ========================================================================*/
/* Generate report
/* ========================================================================*/

var generateReportTimer = null;

function pollReportResult() {
    // Request report file generation
    var reportGenerateRequest = new XMLHttpRequest();
    reportGenerateRequest.open("GET", "/debug/report", true);
    reportGenerateRequest.send(null);
    
    reportGenerateRequest.onreadystatechange = function() {
        if (reportGenerateRequest.readyState == 4) {
            console.log("Report generation finished with " + reportGenerateRequest.status);

            /* 204: the report is not ready yet. */
            if (reportGenerateRequest.status == 204) {
                // Restart poll timer
                generateReportTimer = setTimeout(generateReportTimerTimeout, 1000);
                return;
            }
            
            /* Check if the generation went fine */
            if (reportGenerateRequest.status != 200) {
                console.warn("Report generation finished with error.");
                clearTimeout(generateReportTimer);        
                textArea.value = "Something went wrong :(" + reportGenerateRequest.status;
                button.disabled = false;
                return;
            }
            
            /* The report is finished! Show information and start downloading it. */
            
            /* Stop the timer */
            clearTimeout(generateReportTimer);
            var textArea = document.getElementById("generateReportTextArea");
            var button = document.getElementById("generateReportButton");

        
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

function generateReport() {
    console.log("Requesting to generate report file " + "/debug/report");
    
    var textArea = document.getElementById("generateReportTextArea");
    var button = document.getElementById("generateReportButton");

    button.disabled = true;
    textArea.value = ".";
    
    pollReportResult();

}

function generateReportTimerTimeout() {                    
    var textArea = document.getElementById("generateReportTextArea");
    textArea.value += ".";
    
    pollReportResult();
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
    loadLoggingCategorySettings();
};
