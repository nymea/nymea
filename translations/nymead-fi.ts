<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fi">
<context>
    <name>CloudNotifications</name>
    <message>
        <location filename="../libnymea-core/cloud/cloudnotifications.cpp" line="53"/>
        <location filename="../libnymea-core/cloud/cloudnotifications.cpp" line="118"/>
        <source>Cloud Notifications</source>
        <translation>Pilvi-ilmoitukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/cloud/cloudnotifications.cpp" line="65"/>
        <source>User ID</source>
        <translation>Käyttäjätunnus</translation>
    </message>
    <message>
        <location filename="../libnymea-core/cloud/cloudnotifications.cpp" line="71"/>
        <source>Device</source>
        <translation>Laite</translation>
    </message>
    <message>
        <location filename="../libnymea-core/cloud/cloudnotifications.cpp" line="81"/>
        <source>Title</source>
        <translation>Otsikko</translation>
    </message>
    <message>
        <location filename="../libnymea-core/cloud/cloudnotifications.cpp" line="87"/>
        <source>Message text</source>
        <translation>Viestin teksti</translation>
    </message>
    <message>
        <location filename="../libnymea-core/cloud/cloudnotifications.cpp" line="97"/>
        <source>Send notification</source>
        <translation>Lähetä ilmoitus</translation>
    </message>
    <message>
        <location filename="../libnymea-core/cloud/cloudnotifications.cpp" line="106"/>
        <source>connected</source>
        <translation>yhdistetty</translation>
    </message>
    <message>
        <location filename="../libnymea-core/cloud/cloudnotifications.cpp" line="108"/>
        <source>Connected changed</source>
        <translation>Yhdistetty muutettiin</translation>
    </message>
</context>
<context>
    <name>nymea</name>
    <message>
        <location filename="../server/main.cpp" line="164"/>
        <source>
nymea is an open source IoT (Internet of Things) server, 
which allows to control a lot of different devices from many different 
manufacturers. With the powerful rule engine you are able to connect any 
device available in the system and create individual scenes and behaviors 
for your environment.

</source>
        <translation>
nymea on avoimen lähdekoodin IoT (Internet of Things) -palvelin, joka mahdollistaa useiden valmistajien erilaisten laitteiden hallinnan. Tehokkaalla rule enginella voit yhdistää järjestelmän kaikkiin käytettävissä oleviin laitteisiin ja luoda yksilöllisiä näkymiä ja käytösmalleja ympäristöösi.

</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="176"/>
        <source>Run nymead in the foreground, not as daemon.</source>
        <translation>Aja nymead edustalla, älä daemonina.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="179"/>
        <source>Debug categories to enable. Prefix with &quot;No&quot; to disable. Suffix with &quot;Warnings&quot; to address warnings.
Examples:
-d AWSTraffic
-d NoDeviceManager
-d NoBluetoothWarnings

Categories are:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="191"/>
        <source>Enables all debug categories except *Traffic and *Debug categories. Single debug categories can be disabled again with -d parameter.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="194"/>
        <source>Specify a log file to write to, if this option is not specified, logs will be printed to the standard output.</source>
        <translation>Määritä kirjoitettava lokitiedosto, jos tätä vaihtoehtoa ei ole määritetty, lokit tulostetaan vakiotulostuksena.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="197"/>
        <source>If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus.</source>
        <translation>Jos määritetty, kaikki D-väylä-liittymät yhdistetään istuntoväylään järjestelmäväylän sijaan.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="238"/>
        <source>No such debug category:</source>
        <translation>Ei kyseistä debug-luokkaa:</translation>
    </message>
</context>
<context>
    <name>nymeaserver::DebugServerHandler</name>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="768"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1663"/>
        <source>Debug nymea</source>
        <extracomment>The header title of the debug server interface</extracomment>
        <translation>Debug nymea</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="785"/>
        <source>nymea debug interface</source>
        <extracomment>The main title of the debug server interface</extracomment>
        <translation>nymea debug -liittymä</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="798"/>
        <source>Information</source>
        <extracomment>The name of the section tab in the debug server interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="814"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1495"/>
        <source>Network</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The network section of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="838"/>
        <source>Welcome to the debug interface.</source>
        <extracomment>The welcome message of the debug interface</extracomment>
        <translation>Tervetuloa debug-liittymään.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="839"/>
        <source>This debug interface was designed to provide an easy possibility to get helpful information about the running nymea server.</source>
        <translation>Tämä debug-liittymä suunniteltiin tarjoamaan helppo mahdollisuus saada hyödyllistä tietoa nymea-palvelimen ajamisesta.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="855"/>
        <source>Be aware that this debug interface is a security risk and could offer access to sensible data.</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation>Huomaa, että tämä debug-liittymä on turvallisuusriski ja saattaa tarjota pääsyn arkaluonteisiin tietoihin.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="863"/>
        <source>Server information</source>
        <extracomment>The server information section of the debug interface</extracomment>
        <translation>Palvelintiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="870"/>
        <source>User</source>
        <extracomment>The user name in the server infromation section of the debug interface</extracomment>
        <translation>Käyttäjä</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="876"/>
        <source>Compiled with Qt version</source>
        <extracomment>The Qt build version description in the server infromation section of the debug interface</extracomment>
        <translation>Kootti Qt-versiolla</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="882"/>
        <source>Qt runtime version</source>
        <extracomment>The Qt runtime version description in the server infromation section of the debug interface</extracomment>
        <translation>Qt-ajoaikaversio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="888"/>
        <source>Command</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Käsky</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="897"/>
        <source>Snap name</source>
        <extracomment>The snap name description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-nimi</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="903"/>
        <source>Snap version</source>
        <extracomment>The snap version description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-versio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="909"/>
        <source>Snap directory</source>
        <extracomment>The snap directory description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-hakemisto</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="915"/>
        <source>Snap application data</source>
        <extracomment>The snap application data description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-sovellustiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="921"/>
        <source>Snap user data</source>
        <extracomment>The snap user data description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-käyttäjätiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="927"/>
        <source>Snap common data</source>
        <extracomment>The snap common data description in the server infromation section of the debug interface</extracomment>
        <translation>Snap - yleiset tiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="935"/>
        <source>Server name</source>
        <extracomment>The server name description in the server infromation section of the debug interface</extracomment>
        <translation>Palvelimen nimi</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="941"/>
        <source>Server version</source>
        <extracomment>The server version description in the server infromation section of the debug interface</extracomment>
        <translation>Palvelimen versio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="947"/>
        <source>JSON-RPC version</source>
        <extracomment>The API version description in the server infromation section of the debug interface</extracomment>
        <translation>JSON-RPC-versio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="953"/>
        <source>Language</source>
        <extracomment>The language description in the server infromation section of the debug interface</extracomment>
        <translation>Kieli</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="959"/>
        <source>Timezone</source>
        <extracomment>The timezone description in the server infromation section of the debug interface</extracomment>
        <translation>Aikavyöhyke</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="965"/>
        <source>Server UUID</source>
        <extracomment>The server id description in the server infromation section of the debug interface</extracomment>
        <translation>Palvelin-UUID</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="971"/>
        <source>Settings path</source>
        <extracomment>The settings path description in the server infromation section of the debug interface</extracomment>
        <translation>Asetuspolku</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="977"/>
        <source>Translations path</source>
        <extracomment>The translation path description in the server infromation section of the debug interface</extracomment>
        <translation>Käännöspolku</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="986"/>
        <source>Generate report</source>
        <extracomment>In the server information section of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="989"/>
        <source>If you want to provide all the debug information to a developer, you can generate a report file, which contains all information needed for reproducing a system and get information about possible problems.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1006"/>
        <source>Do not share these generated information public, since they can contain sensible data and should be shared very carefully and only with people you trust!</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1017"/>
        <source>Generate report file</source>
        <extracomment>The generate debug report button text of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1055"/>
        <source>Log database</source>
        <extracomment>The log databse download description of the debug interface</extracomment>
        <translation>Lokitietokanta</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1602"/>
        <source>This section allows you to see the live logs of the nymea server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="806"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1039"/>
        <source>Downloads</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The downloads section of the debug interface</extracomment>
        <translation>Lataukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="822"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1044"/>
        <source>Logs</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The download logs section of the debug interface</extracomment>
        <translation>Lokit</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1077"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1107"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1160"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1210"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1260"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1310"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1360"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1410"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1459"/>
        <source>Download</source>
        <extracomment>The download button description of the debug interface</extracomment>
        <translation>Lataa</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1091"/>
        <source>System logs</source>
        <extracomment>The syslog download description of the debug interface</extracomment>
        <translation>Järjestelmälokit</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1120"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1176"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1226"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1276"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1326"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1376"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1426"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1475"/>
        <source>Show</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1131"/>
        <source>Settings</source>
        <extracomment>The settings download section title of the debug interface</extracomment>
        <translation>Asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1141"/>
        <source>nymead settings</source>
        <extracomment>The nymead settings download description of the debug interface</extracomment>
        <translation>nymead-asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1191"/>
        <source>Device settings</source>
        <extracomment>The device settings download description of the debug interface</extracomment>
        <translation>Laiteasetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1241"/>
        <source>Device states settings</source>
        <extracomment>The device states settings download description of the debug interface</extracomment>
        <translation>Laitetilojen asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1291"/>
        <source>Rules settings</source>
        <extracomment>The rules settings download description of the debug interface</extracomment>
        <translation>Säännöt-asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1341"/>
        <source>Plugins settings</source>
        <extracomment>The plugins settings download description of the debug interface</extracomment>
        <translation>Lisäosien asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1391"/>
        <source>Tag settings</source>
        <extracomment>The tag settings download description of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1440"/>
        <source>MQTT policies</source>
        <extracomment>The MQTT policies download description of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1498"/>
        <source>This section allows you to perform different network connectivity tests in order to find out if the device where nymea is running has full network connectivity.</source>
        <extracomment>The network section description of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1504"/>
        <source>Ping</source>
        <extracomment>The ping section of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1507"/>
        <source>This test makes four ping attempts to the nymea.io server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1516"/>
        <source>Start ping test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1532"/>
        <source>DNS lookup</source>
        <extracomment>The DNS lookup section of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1535"/>
        <source>This test makes a dynamic name server lookup for nymea.io.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1545"/>
        <source>Start DNS lookup test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1560"/>
        <source>Trace path</source>
        <extracomment>The trace section of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1563"/>
        <source>This test showes the trace path from the nymea device to the nymea.io server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1572"/>
        <source>Start trace path test</source>
        <extracomment>The trace path button text of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1599"/>
        <source>Server live logs</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1611"/>
        <source>Start logs</source>
        <extracomment>The connect button for the log stream of the debug interface</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1632"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1706"/>
        <source>Released under the GNU GENERAL PUBLIC LICENSE Version 2.</source>
        <extracomment>The footer license note of the debug interface</extracomment>
        <translation>Julkaistu GNU GENERAL PUBLIC LICENSE, version 2 alla.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1676"/>
        <source>Error  %1</source>
        <extracomment>The HTTP error message of the debug interface. The %1 represents the error code ie.e 404</extracomment>
        <translation>Virhe %1</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="76"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="108"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="139"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="168"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="197"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="226"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="255"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="284"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="314"/>
        <source>Could not find file &quot;%1&quot;.</source>
        <extracomment>The HTTP error message of the debug interface. The %1 represents the file name.</extracomment>
        <translation>Tiedostoa &quot;&amp;1&quot; ei löytynyt.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="85"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="116"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="147"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="176"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="205"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="234"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="263"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="292"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="322"/>
        <source>Could not open file &quot;%1&quot;.</source>
        <extracomment>The HTTP error message of the debug interface. The %1 represents the file name.</extracomment>
        <translation>Tiedostoa &quot;&amp;1&quot; ei voitu avata.</translation>
    </message>
</context>
</TS>
