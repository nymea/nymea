<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="da">
<context>
    <name>ThingManagerImplementation</name>
    <message>
        <location filename="../libnymea-core/integrations/thingmanagerimplementation.cpp" line="309"/>
        <location filename="../libnymea-core/integrations/thingmanagerimplementation.cpp" line="2250"/>
        <source>The plugin for this thing is not loaded.</source>
        <translation>Pluginet til dette element er ikke indlæst.</translation>
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
nymea er en open source IoT-server (&quot;Internet of Things&quot;), som giver mulighed for at styre mange forskellige enheder fra mange forskellige producenter. Med den stærke regelmaskine kan du forbinde alle tilgængelige enheder i systemet og oprette individuelle scenarier og adfærdsregler for dit miljø.

</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="94"/>
        <source>Run nymead in the foreground, not as daemon.</source>
        <translation>Kør nymead i forgrunden, ikke som daemon.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="97"/>
        <source>Disables logging all debug, info and warning categories.</source>
        <translation>Deaktiverer logning af alle debug-, info- og advarselskategorier.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="100"/>
        <source>Enables all info and debug categories except *Traffic and *Debug categories.</source>
        <translation>Aktiverer alle info- og debugkategorier undtagen *Traffic og *Debug.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="103"/>
        <source>Specify a log file to write to, if this option is not specified, logs will be printed to the standard output.</source>
        <translation>Specificér en logfil, der skal skrives til. Hvis denne mulighed ikke er specificeret, udskrives logger til standard-output.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="106"/>
        <source>Log output is colorized by default. Use this option to disable colors.</source>
        <translation>Loguddata er som standard farvelagt. Brug denne indstilling for at deaktivere farver.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="109"/>
        <source>If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus.</source>
        <translation>Hvis det specificeret, vil alle D-Bus grænseflader være bundet til session-bussen i stedet for system-bussen.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="112"/>
        <source>Debug categories to enable. Prefix with &quot;No&quot; to disable. Suffix with &quot;Info&quot; or &quot;Warnings&quot; to address info and warning messages. Enabling a debug category will implicitly enable the according info category.
Examples:
-d ThingManager
-d NoApplicationInfo

</source>
        <translation>Fejlsøgningskategorier der skal aktiveres. Sæt &quot;No&quot; foran for at deaktivere. Tilføj endelserne &quot;Info&quot; eller &quot;Warnings&quot; for at målrette info- og advarselsbeskeder. Når en fejlsøgningskategori aktiveres, aktiveres den tilsvarende infokategori automatisk.
Eksempler:
-d ThingManager
-d NoApplicationInfo
</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="116"/>
        <source>Additional interfaces to listen on. In nymea URI format (e.g. nymeas://127.0.0.2:7777). Note that such interfaces will not require any authentication as they are intended to be used for automated testing only.</source>
        <translation>Yderligere interfaces der skal lyttes på. I nymea-URI-format (f.eks. nymeas://127.0.0.2:7777). Bemærk at sådanne interfaces ikke kræver godkendelse, da de kun er beregnet til automatiserede tests.</translation>
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
        <translation>Debug nymea</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="914"/>
        <source>nymea debug interface</source>
        <extracomment>The main title of the debug server interface</extracomment>
        <translation>nymea debug-grænseflade</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="927"/>
        <source>Information</source>
        <extracomment>The name of the section tab in the debug server interface</extracomment>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="943"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1638"/>
        <source>Network</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The network section of the debug interface</extracomment>
        <translation>Netværk</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="987"/>
        <source>Server information</source>
        <extracomment>The server information section of the debug interface</extracomment>
        <translation>Server-information</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1047"/>
        <source>User</source>
        <extracomment>The user name in the server infromation section of the debug interface</extracomment>
        <translation>Bruger</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1059"/>
        <source>Compiled with Qt version</source>
        <extracomment>The Qt build version description in the server infromation section of the debug interface</extracomment>
        <translation>Kompileret med Qt-version</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1065"/>
        <source>Qt runtime version</source>
        <extracomment>The Qt runtime version description in the server infromation section of the debug interface</extracomment>
        <translation>Qt runtime-version</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1053"/>
        <source>Command</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Command</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1074"/>
        <source>Snap name</source>
        <extracomment>The snap name description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-navn</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1080"/>
        <source>Snap version</source>
        <extracomment>The snap version description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-version</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1086"/>
        <source>Snap directory</source>
        <extracomment>The snap directory description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-mappe</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1092"/>
        <source>Snap application data</source>
        <extracomment>The snap application data description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-applikationsdata</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1098"/>
        <source>Snap user data</source>
        <extracomment>The snap user data description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-brugerdata</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1104"/>
        <source>Snap common data</source>
        <extracomment>The snap common data description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-fællesdata</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="994"/>
        <source>Server name</source>
        <extracomment>The server name description in the server infromation section of the debug interface</extracomment>
        <translation>Servernavn</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1000"/>
        <source>Server version</source>
        <extracomment>The server version description in the server infromation section of the debug interface</extracomment>
        <translation>Server-version</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1006"/>
        <source>JSON-RPC version</source>
        <extracomment>The API version description in the server infromation section of the debug interface</extracomment>
        <translation>JSON-RPC-version</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1012"/>
        <source>Language</source>
        <extracomment>The language description in the server infromation section of the debug interface</extracomment>
        <translation>Sprog</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1023"/>
        <source>Timezone</source>
        <extracomment>The timezone description in the server infromation section of the debug interface</extracomment>
        <translation>Tidszone</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1029"/>
        <source>Server UUID</source>
        <extracomment>The server id description in the server infromation section of the debug interface</extracomment>
        <translation>Server-UUID</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1035"/>
        <source>Settings path</source>
        <extracomment>The settings path description in the server infromation section of the debug interface</extracomment>
        <translation>Indstillingssti</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1041"/>
        <source>Translations path</source>
        <extracomment>The translation path description in the server infromation section of the debug interface</extracomment>
        <translation>Oversættelsessti</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1166"/>
        <source>Generate report</source>
        <extracomment>In the server information section of the debug interface</extracomment>
        <translation>Generér rapport</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1169"/>
        <source>If you want to provide all the debug information to a developer, you can generate a report file, which contains all information needed for reproducing a system and get information about possible problems.</source>
        <translation>Hvis du vil give alle fejlsøgningsoplysninger til en udvikler, kan du generere en rapportfil, der indeholder alle oplysninger, der er nødvendige for at genskabe et system og finde mulige problemer.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1197"/>
        <source>Generate report file</source>
        <extracomment>The generate debug report button text of the debug interface</extracomment>
        <translation>Generér rapportfil</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1334"/>
        <source>Thing settings</source>
        <extracomment>The thing settings download description of the debug interface</extracomment>
        <translation>Thing-indstillinger</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1706"/>
        <source>This test shows the trace path from the nymea device to the nymea.io server.</source>
        <translation>Denne test viser sporingsruten fra nymea-enheden til nymea.io-serveren.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1745"/>
        <source>This section allows you to see the live logs of the nymea server.</source>
        <translation>Denne sektion lader dig se live-loggene fra nymea-serveren.</translation>
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
        <translation>Logger</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="979"/>
        <source>Please note that this debug interface may allow accessing sensitive data about the nymea system and connected devices and services. It is recommended to disable it again when not needed any more.</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation>Bemærk at dette fejlsøgningsinterface kan give adgang til følsomme data om nymea-systemet samt tilsluttede enheder og tjenester. Det anbefales at deaktivere det igen, når det ikke længere behøves.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1115"/>
        <source>System information</source>
        <extracomment>The system information section of the debug interface</extracomment>
        <translation>Systeminformation</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1122"/>
        <source>Hostname</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Værtsnavn</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1133"/>
        <source>Architecture</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Arkitektur</translation>
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
        <translation>Kernelversion</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1151"/>
        <source>Product type</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Produkttype</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1157"/>
        <source>Product version</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Produktversion</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1186"/>
        <source>Please note that the generated debug report may contain sensitive data about the nymea system and connected devices and services.</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation>Bemærk at den genererede fejlsøgningsrapport kan indeholde følsomme data om nymea-systemet samt tilsluttede enheder og tjenester.</translation>
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
        <translation>Download</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1234"/>
        <source>System logs</source>
        <extracomment>The syslog download description of the debug interface</extracomment>
        <translation>System-logger</translation>
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
        <translation>Vis</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1274"/>
        <source>Settings</source>
        <extracomment>The settings download section title of the debug interface</extracomment>
        <translation>Indstillinger</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1284"/>
        <source>nymead settings</source>
        <extracomment>The nymead settings download description of the debug interface</extracomment>
        <translation>nymead-indstillinger</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1384"/>
        <source>Rules settings</source>
        <extracomment>The rules settings download description of the debug interface</extracomment>
        <translation>Indstillinger for regler</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1434"/>
        <source>Plugins settings</source>
        <extracomment>The plugins settings download description of the debug interface</extracomment>
        <translation>Plugin-indstillinger</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1484"/>
        <source>Tag settings</source>
        <extracomment>The tag settings download description of the debug interface</extracomment>
        <translation>Tag-indstillinger</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1533"/>
        <source>MQTT policies</source>
        <extracomment>The MQTT policies download description of the debug interface</extracomment>
        <translation>MQTT-politikker</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1583"/>
        <source>IO Connections</source>
        <extracomment>The MQTT policies download description of the debug interface</extracomment>
        <translation>IO-forbindelser</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1641"/>
        <source>This section allows you to perform different network connectivity tests in order to find out if the device where nymea is running has full network connectivity.</source>
        <extracomment>The network section description of the debug interface</extracomment>
        <translation>Denne sektion gør det muligt at udføre forskellige netværksforbindelsestests for at finde ud af, om enheden der kører nymea har fuld netværksforbindelse.</translation>
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
        <translation>Denne test foretager fire pingforsøg til nymea.io-serveren.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1659"/>
        <source>Start ping test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation>Start pingtest</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1675"/>
        <source>DNS lookup</source>
        <extracomment>The DNS lookup section of the debug interface</extracomment>
        <translation>DNS-opslag</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1678"/>
        <source>This test makes a dynamic name server lookup for nymea.io.</source>
        <translation>Denne test foretager et dynamisk navneopslag for nymea.io.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1688"/>
        <source>Start DNS lookup test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation>Start DNS-opslagstest</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1703"/>
        <source>Trace path</source>
        <extracomment>The trace section of the debug interface</extracomment>
        <translation>Sporingsrute</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1715"/>
        <source>Start trace path test</source>
        <extracomment>The trace path button text of the debug interface</extracomment>
        <translation>Start sporingsrutetest</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1742"/>
        <source>Server live logs</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Server-live-logs</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1757"/>
        <source>Start logs</source>
        <extracomment>The connect button for the log stream of the debug interface</extracomment>
        <translation>Start logs</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1796"/>
        <source>Logging filters</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Logfiltre</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1835"/>
        <source>Logging filters plugins</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Logfiltre for plugins</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1874"/>
        <source>Released under the GNU GENERAL PUBLIC LICENSE Version 3.</source>
        <extracomment>The footer license note of the debug interface</extracomment>
        <translation>Udgivet under GNU GENERAL PUBLIC LICENSE Version 3.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1948"/>
        <source>Released under the GNU GENERAL PUBLIC LICENSE Version 2.</source>
        <translation>Udgivet i henhold til GNU GENERAL PUBLIC LICENSE Version 2.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1918"/>
        <source>Error  %1</source>
        <extracomment>The HTTP error message of the debug interface. The %1 represents the error code ie.e 404</extracomment>
        <translation>Fejl %1</translation>
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
        <translation>Kunne ikke finde filen &quot;%1&quot;.</translation>
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
        <translation>Kunne ikke åbne filen &quot;%1&quot;.</translation>
    </message>
</context>
</TS>
