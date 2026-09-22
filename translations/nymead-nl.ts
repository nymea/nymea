<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="nl">
<context>
    <name>ThingManagerImplementation</name>
    <message>
        <location filename="../libnymea-core/integrations/thingmanagerimplementation.cpp" line="309"/>
        <location filename="../libnymea-core/integrations/thingmanagerimplementation.cpp" line="2250"/>
        <source>The plugin for this thing is not loaded.</source>
        <translation>De plugin voor dit ding is niet geladen.</translation>
    </message>
</context>
<context>
    <name>nymea</name>
    <message>
        <location filename="../server/main.cpp" line="82"/>
        <source>
nymea is an open source IoT (Internet of Things) server, 
which allows to control a lot of different devices from many different 
manufacturers. With the powerful rule engine you are able to connect any 
device available in the system and create individual scenes and behaviors 
for your environment.

</source>
        <translation>
nymea is een open source IoT-server (IoT: Internet der dingen), 
waarmee je veel verschillende apparaten van veel verschillende fabrikanten 
kunt bedienen. Met het krachtige regelsysteem kun je elk beschikbaar apparaat 
binnen het systeem verbinden en individuele scenes en acties voor je omgeving 
instellen.

</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="94"/>
        <source>Run nymead in the foreground, not as daemon.</source>
        <translation>Voer nymead op de voorgrond uit en niet als daemon.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="97"/>
        <source>Disables logging all debug, info and warning categories.</source>
        <translation>Schakel het bijhouden van logs uit voor alle debug-, informatie- en waarschuwingscategorieën.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="100"/>
        <source>Enables all info and debug categories except *Traffic and *Debug categories.</source>
        <translation>Schakel alle informatie- en debugcategorieën in, behalve de *Traffic en *Debug categorieën.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="103"/>
        <source>Specify a log file to write to, if this option is not specified, logs will be printed to the standard output.</source>
        <translation>Geef aan naar welk logbestand geschreven moet worden. Als er geen logbestand is aangegeven, worden logbestanden naar de standaarduitvoer geschreven.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="106"/>
        <source>Log output is colorized by default. Use this option to disable colors.</source>
        <translation>De output van de logs wordt met kleuren weergegeven. Gebruik deze optie om kleuren uit te schakelen.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="109"/>
        <source>If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus.</source>
        <translation>Indien aangegeven, worden alle D-businterfaces gebonden aan de sessiebus, en niet aan de systeembus.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="112"/>
        <source>Debug categories to enable. Prefix with &quot;No&quot; to disable. Suffix with &quot;Info&quot; or &quot;Warnings&quot; to address info and warning messages. Enabling a debug category will implicitly enable the according info category.
Examples:
-d ThingManager
-d NoApplicationInfo

</source>
        <translation>Schakel hier debugcategorieën in of uit. Gebruik de prefix &quot;No&quot; om een categorie uit te schakelen. Gebruik de suffix &quot;Info&quot; voor informatieberichten en de suffix &quot;Warnings&quot; voor waarschuwingen. Als je en debugcategorie inschakelt, wordt de bijbehorende informatiecategorie automatisch ook ingeschakeld.
Voorbeelden:
-d ThingManager
-d NoApplicationInfo

</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="116"/>
        <source>Additional interfaces to listen on. In nymea URI format (e.g. nymeas://127.0.0.2:7777). Note that such interfaces will not require any authentication as they are intended to be used for automated testing only.</source>
        <translation>Extra interfaces om in de gaten te houden. In nymea URI format (bijvoorbeeld: nymeas://127.0.0.2:7777). Let op: zulke interfaces vereisen geen authenticatie omdat ze alleen bedoeld zijn voor automatisch testen.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="119"/>
        <source>Disable the influx DB log engine.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="122"/>
        <source>Uses the given &lt;path&gt; for storing configurations and persistent data. Using this option will override the NYMEA_CONFIG_PATH environment variable.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>nymeaserver::DebugServerHandler</name>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="897"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1905"/>
        <source>Debug nymea</source>
        <extracomment>The header title of the debug server interface</extracomment>
        <translation>nymea debuggen</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="914"/>
        <source>nymea debug interface</source>
        <extracomment>The main title of the debug server interface</extracomment>
        <translation>nymea debug-interface</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="927"/>
        <source>Information</source>
        <extracomment>The name of the section tab in the debug server interface</extracomment>
        <translation>Informatie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="943"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1638"/>
        <source>Network</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The network section of the debug interface</extracomment>
        <translation>Netwerk</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="987"/>
        <source>Server information</source>
        <extracomment>The server information section of the debug interface</extracomment>
        <translation>Serverinformatie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1047"/>
        <source>User</source>
        <extracomment>The user name in the server infromation section of the debug interface</extracomment>
        <translation>Gebruiker</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1059"/>
        <source>Compiled with Qt version</source>
        <extracomment>The Qt build version description in the server infromation section of the debug interface</extracomment>
        <translation>Gecompileerd met Qt-versie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1065"/>
        <source>Qt runtime version</source>
        <extracomment>The Qt runtime version description in the server infromation section of the debug interface</extracomment>
        <translation>Qt-runtimeversie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1053"/>
        <source>Command</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Opdracht</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1074"/>
        <source>Snap name</source>
        <extracomment>The snap name description in the server infromation section of the debug interface</extracomment>
        <translation>Snapnaam</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1080"/>
        <source>Snap version</source>
        <extracomment>The snap version description in the server infromation section of the debug interface</extracomment>
        <translation>Snapversie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1086"/>
        <source>Snap directory</source>
        <extracomment>The snap directory description in the server infromation section of the debug interface</extracomment>
        <translation>Snapdirectory</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1092"/>
        <source>Snap application data</source>
        <extracomment>The snap application data description in the server infromation section of the debug interface</extracomment>
        <translation>Toepassingsgegevens voor Snap</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1098"/>
        <source>Snap user data</source>
        <extracomment>The snap user data description in the server infromation section of the debug interface</extracomment>
        <translation>Gebruikersgegevens voor Snap</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1104"/>
        <source>Snap common data</source>
        <extracomment>The snap common data description in the server infromation section of the debug interface</extracomment>
        <translation>Gemeenschappelijke gegevens voor Snap</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="994"/>
        <source>Server name</source>
        <extracomment>The server name description in the server infromation section of the debug interface</extracomment>
        <translation>Servernaam</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1000"/>
        <source>Server version</source>
        <extracomment>The server version description in the server infromation section of the debug interface</extracomment>
        <translation>Serverversie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1006"/>
        <source>JSON-RPC version</source>
        <extracomment>The API version description in the server infromation section of the debug interface</extracomment>
        <translation>JSON-RPC-versie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1012"/>
        <source>Language</source>
        <extracomment>The language description in the server infromation section of the debug interface</extracomment>
        <translation>Taal</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1023"/>
        <source>Timezone</source>
        <extracomment>The timezone description in the server infromation section of the debug interface</extracomment>
        <translation>Tijdzone</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1029"/>
        <source>Server UUID</source>
        <extracomment>The server id description in the server infromation section of the debug interface</extracomment>
        <translation>Server UUID</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1035"/>
        <source>Settings path</source>
        <extracomment>The settings path description in the server infromation section of the debug interface</extracomment>
        <translation>Pad naar instellingen</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1041"/>
        <source>Translations path</source>
        <extracomment>The translation path description in the server infromation section of the debug interface</extracomment>
        <translation>Pad naar vertalingen</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1166"/>
        <source>Generate report</source>
        <extracomment>In the server information section of the debug interface</extracomment>
        <translation>Maak een rapport</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1169"/>
        <source>If you want to provide all the debug information to a developer, you can generate a report file, which contains all information needed for reproducing a system and get information about possible problems.</source>
        <translation>Als je alle debuginformatie naar een ontwikkelaar wilt sturen, kun je een rapportagebestand maken. Hier staat alle informatie in om een systeem na te maken en om informatie over mogelijke problemen te achterhalen.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1197"/>
        <source>Generate report file</source>
        <extracomment>The generate debug report button text of the debug interface</extracomment>
        <translation>Maak een rapportagebestand</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1334"/>
        <source>Thing settings</source>
        <extracomment>The thing settings download description of the debug interface</extracomment>
        <translation>Ding instellingen</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1706"/>
        <source>This test shows the trace path from the nymea device to the nymea.io server.</source>
        <translation>Deze test toont het tracé van het nymea apparaat naar de nymea.io server.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1745"/>
        <source>This section allows you to see the live logs of the nymea server.</source>
        <translation>In dit gedeelte kun je alle live logs van de nymea server bekijken.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="935"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1219"/>
        <source>Downloads</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The downloads section of the debug interface</extracomment>
        <translation>Downloads</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="951"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1224"/>
        <source>Logs</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The download logs section of the debug interface</extracomment>
        <translation>Logs</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="979"/>
        <source>Please note that this debug interface may allow accessing sensitive data about the nymea system and connected devices and services. It is recommended to disable it again when not needed any more.</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation>Let op: deze debug interface kan toegang geven tot gevoelige gegevens over het nymea systeem en de verbonden apparaten en services. Schakel de debug interface weer uit zodra je hem niet meer nodig hebt.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1115"/>
        <source>System information</source>
        <extracomment>The system information section of the debug interface</extracomment>
        <translation>Systeeminformatie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1122"/>
        <source>Hostname</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Hostnaam</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1133"/>
        <source>Architecture</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Architectuur</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1139"/>
        <source>Kernel type</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Kerneltype</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1145"/>
        <source>Kernel version</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Kernelversie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1151"/>
        <source>Product type</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Producttype</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1157"/>
        <source>Product version</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Productversie</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1186"/>
        <source>Please note that the generated debug report may contain sensitive data about the nymea system and connected devices and services.</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation>Let op: het gemaakte debugrapport kan gevoelige informatie bevatten over het nymea systeem en de verbonden apparaten en services.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1250"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1303"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1353"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1403"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1453"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1503"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1552"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1602"/>
        <source>Download</source>
        <translation>Downloaden</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1234"/>
        <source>System logs</source>
        <extracomment>The syslog download description of the debug interface</extracomment>
        <translation>Systeemlogs</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1263"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1319"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1369"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1419"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1469"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1519"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1568"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1618"/>
        <source>Show</source>
        <translation>Toon</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1274"/>
        <source>Settings</source>
        <extracomment>The settings download section title of the debug interface</extracomment>
        <translation>Instellingen</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1284"/>
        <source>nymead settings</source>
        <extracomment>The nymead settings download description of the debug interface</extracomment>
        <translation>Instellingen van nymead</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1384"/>
        <source>Rules settings</source>
        <extracomment>The rules settings download description of the debug interface</extracomment>
        <translation>Regelinstellingen</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1434"/>
        <source>Plugins settings</source>
        <extracomment>The plugins settings download description of the debug interface</extracomment>
        <translation>Plugininstellingen</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1484"/>
        <source>Tag settings</source>
        <extracomment>The tag settings download description of the debug interface</extracomment>
        <translation>Taginstellingen</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1533"/>
        <source>MQTT policies</source>
        <extracomment>The MQTT policies download description of the debug interface</extracomment>
        <translation>MQTT-beleid</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1583"/>
        <source>IO Connections</source>
        <extracomment>The MQTT policies download description of the debug interface</extracomment>
        <translation>IO-verbindingen</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1641"/>
        <source>This section allows you to perform different network connectivity tests in order to find out if the device where nymea is running has full network connectivity.</source>
        <extracomment>The network section description of the debug interface</extracomment>
        <translation>In dit gedeelte kun je verschillende verbindingstests op het netwerk uitvoeren. Zo kun je achterhalen of het apparaat waar nymea op is geïnstalleerd volledig verbinding met het netwerk maakt.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1647"/>
        <source>Ping</source>
        <extracomment>The ping section of the debug interface</extracomment>
        <translation>Ping</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1650"/>
        <source>This test makes four ping attempts to the nymea.io server.</source>
        <translation>Deze test probeert de nymea.io server vier keer te pingen.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1659"/>
        <source>Start ping test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation>Start de ping test</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1675"/>
        <source>DNS lookup</source>
        <extracomment>The DNS lookup section of the debug interface</extracomment>
        <translation>DNS lookup</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1678"/>
        <source>This test makes a dynamic name server lookup for nymea.io.</source>
        <translation>Deze test maakt een dynamische name server lookup voor nymea.io.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1688"/>
        <source>Start DNS lookup test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation>Start de DNS lookup test</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1703"/>
        <source>Trace path</source>
        <extracomment>The trace section of the debug interface</extracomment>
        <translation>Tracé</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1715"/>
        <source>Start trace path test</source>
        <extracomment>The trace path button text of the debug interface</extracomment>
        <translation>Start de tracé test</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1742"/>
        <source>Server live logs</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Live logs van de server</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1757"/>
        <source>Start logs</source>
        <extracomment>The connect button for the log stream of the debug interface</extracomment>
        <translation>Start de logs</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1796"/>
        <source>Logging filters</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Filters voor de logs</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1835"/>
        <source>Logging filters plugins</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Plugins voor de logfilters</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1874"/>
        <source>Released under the GNU GENERAL PUBLIC LICENSE Version 3.</source>
        <extracomment>The footer license note of the debug interface</extracomment>
        <translation>Uitgebracht onder de GNU GENERAL PUBLIC LICENSE versie 3.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1948"/>
        <source>Released under the GNU GENERAL PUBLIC LICENSE Version 2.</source>
        <translation>Uitgebracht onder de GNU GENERAL PUBLIC LICENSE versie 2.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1918"/>
        <source>Error  %1</source>
        <extracomment>The HTTP error message of the debug interface. The %1 represents the error code ie.e 404</extracomment>
        <translation>Foutcode %1</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="97"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="128"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="157"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="186"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="216"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="245"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="275"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="304"/>
        <source>Could not find file &quot;%1&quot;.</source>
        <translation>Kan bestand &quot;%1&quot; niet vinden.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="105"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="136"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="165"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="194"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="224"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="253"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="283"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="312"/>
        <source>Could not open file &quot;%1&quot;.</source>
        <translation>Kan bestand &quot;%1&quot; niet openen.</translation>
    </message>
</context>
</TS>
