<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="it">
<context>
    <name>ThingManagerImplementation</name>
    <message>
        <location filename="../libnymea-core/integrations/thingmanagerimplementation.cpp" line="309"/>
        <location filename="../libnymea-core/integrations/thingmanagerimplementation.cpp" line="2250"/>
        <source>The plugin for this thing is not loaded.</source>
        <translation>Il plugin per questo elemento non è caricato.</translation>
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
nymea è un server IoT (Internet of Things) open source, che consente di controllare molti dispositivi diversi di diversi produttori. Con il potente motore di regole puoi connettere qualsiasi dispositivo disponibile nel sistema e creare scene e comportamenti individuali per il tuo ambiente.

</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="94"/>
        <source>Run nymead in the foreground, not as daemon.</source>
        <translation>Esegui nymead in primo piano, non come demone.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="97"/>
        <source>Disables logging all debug, info and warning categories.</source>
        <translation>Disabilita la registrazione di tutte le categorie di debug, info e avvisi.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="100"/>
        <source>Enables all info and debug categories except *Traffic and *Debug categories.</source>
        <translation>Abilita tutte le categorie di info e debug tranne le categorie *Traffic e *Debug.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="103"/>
        <source>Specify a log file to write to, if this option is not specified, logs will be printed to the standard output.</source>
        <translation>Specificare un file di registro in cui scrivere, se questa opzione non è specificata, i registri verranno stampati sullo standard output.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="106"/>
        <source>Log output is colorized by default. Use this option to disable colors.</source>
        <translation>L&apos;output del log è colorato per impostazione predefinita. Usa questa opzione per disabilitare i colori.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="109"/>
        <source>If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus.</source>
        <translation>Se specificato, tutte le interfacce bus D saranno associate al bus di sessione anziché al bus di sistema.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="112"/>
        <source>Debug categories to enable. Prefix with &quot;No&quot; to disable. Suffix with &quot;Info&quot; or &quot;Warnings&quot; to address info and warning messages. Enabling a debug category will implicitly enable the according info category.
Examples:
-d ThingManager
-d NoApplicationInfo

</source>
        <translation>Categorie di debug da abilitare. Anteponi &quot;No&quot; per disabilitarle. Aggiungi il suffisso &quot;Info&quot; o &quot;Warnings&quot; per gestire i messaggi informativi e di avviso. Abilitare una categoria di debug abilita implicitamente anche la relativa categoria info.
Esempi:
-d ThingManager
-d NoApplicationInfo
</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="116"/>
        <source>Additional interfaces to listen on. In nymea URI format (e.g. nymeas://127.0.0.2:7777). Note that such interfaces will not require any authentication as they are intended to be used for automated testing only.</source>
        <translation>Interfacce aggiuntive da cui ascoltare. In formato URI di nymea (es. nymeas://127.0.0.2:7777). Nota che tali interfacce non richiederanno autenticazione perché pensate solo per i test automatizzati.</translation>
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
        <translation>debug nymea</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="914"/>
        <source>nymea debug interface</source>
        <extracomment>The main title of the debug server interface</extracomment>
        <translation>interfaccia di debug nymea</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="927"/>
        <source>Information</source>
        <extracomment>The name of the section tab in the debug server interface</extracomment>
        <translation>Informazioni</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="943"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1638"/>
        <source>Network</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The network section of the debug interface</extracomment>
        <translation>Rete</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="987"/>
        <source>Server information</source>
        <extracomment>The server information section of the debug interface</extracomment>
        <translation>Informazioni sul sever</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1047"/>
        <source>User</source>
        <extracomment>The user name in the server infromation section of the debug interface</extracomment>
        <translation>Utente</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1059"/>
        <source>Compiled with Qt version</source>
        <extracomment>The Qt build version description in the server infromation section of the debug interface</extracomment>
        <translation>Compilato con la versione Qt</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1065"/>
        <source>Qt runtime version</source>
        <extracomment>The Qt runtime version description in the server infromation section of the debug interface</extracomment>
        <translation>Versione runtime Qt</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1053"/>
        <source>Command</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Comando</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1074"/>
        <source>Snap name</source>
        <extracomment>The snap name description in the server infromation section of the debug interface</extracomment>
        <translation>Nome snap</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1080"/>
        <source>Snap version</source>
        <extracomment>The snap version description in the server infromation section of the debug interface</extracomment>
        <translation>Versione snap</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1086"/>
        <source>Snap directory</source>
        <extracomment>The snap directory description in the server infromation section of the debug interface</extracomment>
        <translation>Directory snap</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1092"/>
        <source>Snap application data</source>
        <extracomment>The snap application data description in the server infromation section of the debug interface</extracomment>
        <translation>Dati applicativo snap</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1098"/>
        <source>Snap user data</source>
        <extracomment>The snap user data description in the server infromation section of the debug interface</extracomment>
        <translation>Dati utente snap</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1104"/>
        <source>Snap common data</source>
        <extracomment>The snap common data description in the server infromation section of the debug interface</extracomment>
        <translation>Dati comuni snap</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="994"/>
        <source>Server name</source>
        <extracomment>The server name description in the server infromation section of the debug interface</extracomment>
        <translation>Nome del server</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1000"/>
        <source>Server version</source>
        <extracomment>The server version description in the server infromation section of the debug interface</extracomment>
        <translation>Versione del server</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1006"/>
        <source>JSON-RPC version</source>
        <extracomment>The API version description in the server infromation section of the debug interface</extracomment>
        <translation>Versione JSON-RPC</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1012"/>
        <source>Language</source>
        <extracomment>The language description in the server infromation section of the debug interface</extracomment>
        <translation>Lingua</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1023"/>
        <source>Timezone</source>
        <extracomment>The timezone description in the server infromation section of the debug interface</extracomment>
        <translation>Fuso orario</translation>
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
        <translation>Percorso delle impostazioni</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1041"/>
        <source>Translations path</source>
        <extracomment>The translation path description in the server infromation section of the debug interface</extracomment>
        <translation>Percorso delle traduzioni</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1166"/>
        <source>Generate report</source>
        <extracomment>In the server information section of the debug interface</extracomment>
        <translation>Genera rapporto</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1169"/>
        <source>If you want to provide all the debug information to a developer, you can generate a report file, which contains all information needed for reproducing a system and get information about possible problems.</source>
        <translation>Se vuoi fornire tutte le informazioni di debug a uno sviluppatore, puoi generare un file di rapporto che contiene tutto il necessario per riprodurre un sistema e individuare eventuali problemi.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1197"/>
        <source>Generate report file</source>
        <extracomment>The generate debug report button text of the debug interface</extracomment>
        <translation>Genera file di rapporto</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1334"/>
        <source>Thing settings</source>
        <extracomment>The thing settings download description of the debug interface</extracomment>
        <translation>Impostazioni dell&apos;oggetto</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1706"/>
        <source>This test shows the trace path from the nymea device to the nymea.io server.</source>
        <translation>Questo test mostra il percorso di traccia dal dispositivo nymea al server nymea.io.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1745"/>
        <source>This section allows you to see the live logs of the nymea server.</source>
        <translation>Questa sezione consente di vedere i log in tempo reale del server nymea.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="935"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1219"/>
        <source>Downloads</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The downloads section of the debug interface</extracomment>
        <translation>Download</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="951"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1224"/>
        <source>Logs</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The download logs section of the debug interface</extracomment>
        <translation>Log</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="979"/>
        <source>Please note that this debug interface may allow accessing sensitive data about the nymea system and connected devices and services. It is recommended to disable it again when not needed any more.</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation>Tieni presente che questa interfaccia di debug può consentire l&apos;accesso a dati sensibili sul sistema nymea e sui dispositivi e servizi collegati. Si consiglia di disabilitarla nuovamente quando non è più necessaria.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1115"/>
        <source>System information</source>
        <extracomment>The system information section of the debug interface</extracomment>
        <translation>Informazioni di sistema</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1122"/>
        <source>Hostname</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Nome host</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1133"/>
        <source>Architecture</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Architettura</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1139"/>
        <source>Kernel type</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Tipo di kernel</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1145"/>
        <source>Kernel version</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Versione del kernel</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1151"/>
        <source>Product type</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Tipo di prodotto</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1157"/>
        <source>Product version</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Versione del prodotto</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1186"/>
        <source>Please note that the generated debug report may contain sensitive data about the nymea system and connected devices and services.</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation>Tieni presente che il rapporto di debug generato può contenere dati sensibili sul sistema nymea e sui dispositivi e servizi collegati.</translation>
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
        <translation>Log di sistema</translation>
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
        <translation>Mostra</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1274"/>
        <source>Settings</source>
        <extracomment>The settings download section title of the debug interface</extracomment>
        <translation>Impostazioni</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1284"/>
        <source>nymead settings</source>
        <extracomment>The nymead settings download description of the debug interface</extracomment>
        <translation>Impostazioni nymead</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1384"/>
        <source>Rules settings</source>
        <extracomment>The rules settings download description of the debug interface</extracomment>
        <translation>Impostazioni delle regole</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1434"/>
        <source>Plugins settings</source>
        <extracomment>The plugins settings download description of the debug interface</extracomment>
        <translation>Impostazioni dei plugin</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1484"/>
        <source>Tag settings</source>
        <extracomment>The tag settings download description of the debug interface</extracomment>
        <translation>Impostazioni tag</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1533"/>
        <source>MQTT policies</source>
        <extracomment>The MQTT policies download description of the debug interface</extracomment>
        <translation>Criteri MQTT</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1583"/>
        <source>IO Connections</source>
        <extracomment>The MQTT policies download description of the debug interface</extracomment>
        <translation>Connessioni IO</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1641"/>
        <source>This section allows you to perform different network connectivity tests in order to find out if the device where nymea is running has full network connectivity.</source>
        <extracomment>The network section description of the debug interface</extracomment>
        <translation>Questa sezione consente di eseguire diversi test di connettività di rete per verificare se il dispositivo su cui gira nymea dispone di piena connettività.</translation>
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
        <translation>Questo test esegue quattro tentativi di ping al server nymea.io.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1659"/>
        <source>Start ping test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation>Avvia test di ping</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1675"/>
        <source>DNS lookup</source>
        <extracomment>The DNS lookup section of the debug interface</extracomment>
        <translation>Ricerca DNS</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1678"/>
        <source>This test makes a dynamic name server lookup for nymea.io.</source>
        <translation>Questo test esegue una ricerca dinamica del nome per nymea.io.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1688"/>
        <source>Start DNS lookup test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation>Avvia test di ricerca DNS</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1703"/>
        <source>Trace path</source>
        <extracomment>The trace section of the debug interface</extracomment>
        <translation>Traccia percorso</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1715"/>
        <source>Start trace path test</source>
        <extracomment>The trace path button text of the debug interface</extracomment>
        <translation>Avvia test di tracciamento del percorso</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1742"/>
        <source>Server live logs</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Log live del server</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1757"/>
        <source>Start logs</source>
        <extracomment>The connect button for the log stream of the debug interface</extracomment>
        <translation>Avvia log</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1796"/>
        <source>Logging filters</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Filtri di log</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1835"/>
        <source>Logging filters plugins</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Filtri di log dei plugin</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1874"/>
        <source>Released under the GNU GENERAL PUBLIC LICENSE Version 3.</source>
        <extracomment>The footer license note of the debug interface</extracomment>
        <translation>Rilasciato sotto la GNU GENERAL PUBLIC LICENSE Versione 3.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1948"/>
        <source>Released under the GNU GENERAL PUBLIC LICENSE Version 2.</source>
        <translation>Rilasciato su licenza GNU General Public License versione 2.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1918"/>
        <source>Error  %1</source>
        <extracomment>The HTTP error message of the debug interface. The %1 represents the error code ie.e 404</extracomment>
        <translation>Errore %1</translation>
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
        <translation>File &quot;%1&quot; non trovato.</translation>
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
        <translation>Impossibile aprire il file &quot;%1&quot;.</translation>
    </message>
</context>
</TS>
