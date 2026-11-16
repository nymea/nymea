<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fi">
<context>
    <name>ThingManagerImplementation</name>
    <message>
        <location filename="../libnymea-core/integrations/thingmanagerimplementation.cpp" line="309"/>
        <location filename="../libnymea-core/integrations/thingmanagerimplementation.cpp" line="2250"/>
        <source>The plugin for this thing is not loaded.</source>
        <translation>Tämän laitteen liitännäistä ei ole ladattu.</translation>
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
nymea on avoimen lähdekoodin IoT (Internet of Things) -palvelin, joka mahdollistaa useiden valmistajien erilaisten laitteiden hallinnan. Tehokkaalla rule enginella voit yhdistää järjestelmän kaikkiin käytettävissä oleviin laitteisiin ja luoda yksilöllisiä näkymiä ja käytösmalleja ympäristöösi.

</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="94"/>
        <source>Run nymead in the foreground, not as daemon.</source>
        <translation>Aja nymead edustalla, älä daemonina.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="97"/>
        <source>Disables logging all debug, info and warning categories.</source>
        <translation>Poistaa käytöstä kaikkien vianetsintä-, info- ja varoitusluokkien lokituksen.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="100"/>
        <source>Enables all info and debug categories except *Traffic and *Debug categories.</source>
        <translation>Ottaa käyttöön kaikki info- ja vianetsintäluokat lukuun ottamatta *Traffic- ja *Debug-luokkia.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="103"/>
        <source>Specify a log file to write to, if this option is not specified, logs will be printed to the standard output.</source>
        <translation>Määritä kirjoitettava lokitiedosto, jos tätä vaihtoehtoa ei ole määritetty, lokit tulostetaan vakiotulostuksena.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="106"/>
        <source>Log output is colorized by default. Use this option to disable colors.</source>
        <translation>Lokituloste on oletuksena väritetty. Käytä tätä asetusta värien poistamiseen.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="109"/>
        <source>If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus.</source>
        <translation>Jos määritetty, kaikki D-väylä-liittymät yhdistetään istuntoväylään järjestelmäväylän sijaan.</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="112"/>
        <source>Debug categories to enable. Prefix with &quot;No&quot; to disable. Suffix with &quot;Info&quot; or &quot;Warnings&quot; to address info and warning messages. Enabling a debug category will implicitly enable the according info category.
Examples:
-d ThingManager
-d NoApplicationInfo

</source>
        <translation>Käyttöön otettavat vianetsintäluokat. Lisää etuliite &quot;No&quot; poistaaksesi ne käytöstä. Lisää jälkiliite &quot;Info&quot; tai &quot;Warnings&quot; kohdistamaan tieto- ja varoitusviesteihin. Kun vianetsintäluokka otetaan käyttöön, vastaava infoluokka otetaan käyttöön automaattisesti.
Esimerkit:
-d ThingManager
-d NoApplicationInfo
</translation>
    </message>
    <message>
        <location filename="../server/main.cpp" line="116"/>
        <source>Additional interfaces to listen on. In nymea URI format (e.g. nymeas://127.0.0.2:7777). Note that such interfaces will not require any authentication as they are intended to be used for automated testing only.</source>
        <translation>Lisärajapinnat, joita kuunnellaan. Nymea-URI-muodossa (esim. nymeas://127.0.0.2:7777). Huomaa, että tällaiset rajapinnat eivät vaadi todennusta, koska ne on tarkoitettu vain automatisoitua testausta varten.</translation>
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
        <translation>nymea debug -liittymä</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="927"/>
        <source>Information</source>
        <extracomment>The name of the section tab in the debug server interface</extracomment>
        <translation>Tiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="943"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1638"/>
        <source>Network</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The network section of the debug interface</extracomment>
        <translation>Verkko</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="987"/>
        <source>Server information</source>
        <extracomment>The server information section of the debug interface</extracomment>
        <translation>Palvelintiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1047"/>
        <source>User</source>
        <extracomment>The user name in the server infromation section of the debug interface</extracomment>
        <translation>Käyttäjä</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1059"/>
        <source>Compiled with Qt version</source>
        <extracomment>The Qt build version description in the server infromation section of the debug interface</extracomment>
        <translation>Kootti Qt-versiolla</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1065"/>
        <source>Qt runtime version</source>
        <extracomment>The Qt runtime version description in the server infromation section of the debug interface</extracomment>
        <translation>Qt-ajoaikaversio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1053"/>
        <source>Command</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Käsky</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1074"/>
        <source>Snap name</source>
        <extracomment>The snap name description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-nimi</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1080"/>
        <source>Snap version</source>
        <extracomment>The snap version description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-versio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1086"/>
        <source>Snap directory</source>
        <extracomment>The snap directory description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-hakemisto</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1092"/>
        <source>Snap application data</source>
        <extracomment>The snap application data description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-sovellustiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1098"/>
        <source>Snap user data</source>
        <extracomment>The snap user data description in the server infromation section of the debug interface</extracomment>
        <translation>Snap-käyttäjätiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1104"/>
        <source>Snap common data</source>
        <extracomment>The snap common data description in the server infromation section of the debug interface</extracomment>
        <translation>Snap - yleiset tiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="994"/>
        <source>Server name</source>
        <extracomment>The server name description in the server infromation section of the debug interface</extracomment>
        <translation>Palvelimen nimi</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1000"/>
        <source>Server version</source>
        <extracomment>The server version description in the server infromation section of the debug interface</extracomment>
        <translation>Palvelimen versio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1006"/>
        <source>JSON-RPC version</source>
        <extracomment>The API version description in the server infromation section of the debug interface</extracomment>
        <translation>JSON-RPC-versio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1012"/>
        <source>Language</source>
        <extracomment>The language description in the server infromation section of the debug interface</extracomment>
        <translation>Kieli</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1023"/>
        <source>Timezone</source>
        <extracomment>The timezone description in the server infromation section of the debug interface</extracomment>
        <translation>Aikavyöhyke</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1029"/>
        <source>Server UUID</source>
        <extracomment>The server id description in the server infromation section of the debug interface</extracomment>
        <translation>Palvelin-UUID</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1035"/>
        <source>Settings path</source>
        <extracomment>The settings path description in the server infromation section of the debug interface</extracomment>
        <translation>Asetuspolku</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1041"/>
        <source>Translations path</source>
        <extracomment>The translation path description in the server infromation section of the debug interface</extracomment>
        <translation>Käännöspolku</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1166"/>
        <source>Generate report</source>
        <extracomment>In the server information section of the debug interface</extracomment>
        <translation>Luo raportti</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1169"/>
        <source>If you want to provide all the debug information to a developer, you can generate a report file, which contains all information needed for reproducing a system and get information about possible problems.</source>
        <translation>Jos haluat toimittaa kaikki vianetsintätiedot kehittäjälle, voit luoda raporttitiedoston, joka sisältää kaiken tiedon, jota tarvitaan järjestelmän toistamiseen ja mahdollisten ongelmien löytämiseen.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1197"/>
        <source>Generate report file</source>
        <extracomment>The generate debug report button text of the debug interface</extracomment>
        <translation>Luo raporttitiedosto</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1334"/>
        <source>Thing settings</source>
        <extracomment>The thing settings download description of the debug interface</extracomment>
        <translation>Thing-asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1706"/>
        <source>This test shows the trace path from the nymea device to the nymea.io server.</source>
        <translation>Tämä testi näyttää jäljityspolun nymea-laitteesta nymea.io-palvelimelle.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1745"/>
        <source>This section allows you to see the live logs of the nymea server.</source>
        <translation>Tämä osio antaa nähdä nymea-palvelimen live-lokit.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="935"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1219"/>
        <source>Downloads</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The downloads section of the debug interface</extracomment>
        <translation>Lataukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="951"/>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1224"/>
        <source>Logs</source>
        <extracomment>The name of the section tab in the debug server interface
----------
The download logs section of the debug interface</extracomment>
        <translation>Lokit</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="979"/>
        <source>Please note that this debug interface may allow accessing sensitive data about the nymea system and connected devices and services. It is recommended to disable it again when not needed any more.</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation>Huomaa, että tämä vianetsintärajapinta voi mahdollistaa pääsyn arkaluonteisiin tietoihin nymea-järjestelmästä sekä liitetyistä laitteista ja palveluista. On suositeltavaa ottaa se pois käytöstä, kun sitä ei enää tarvita.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1115"/>
        <source>System information</source>
        <extracomment>The system information section of the debug interface</extracomment>
        <translation>Järjestelmätiedot</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1122"/>
        <source>Hostname</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Isäntänimi</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1133"/>
        <source>Architecture</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Arkkitehtuuri</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1139"/>
        <source>Kernel type</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Ytimen tyyppi</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1145"/>
        <source>Kernel version</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Ytimen versio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1151"/>
        <source>Product type</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Tuotetyyppi</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1157"/>
        <source>Product version</source>
        <extracomment>The command description in the server infromation section of the debug interface</extracomment>
        <translation>Tuoteversio</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1186"/>
        <source>Please note that the generated debug report may contain sensitive data about the nymea system and connected devices and services.</source>
        <extracomment>The warning message of the debug interface</extracomment>
        <translation>Huomaa, että luotu vianetsintäraportti voi sisältää arkaluonteisia tietoja nymea-järjestelmästä sekä liitetyistä laitteista ja palveluista.</translation>
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
        <translation>Lataa</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1234"/>
        <source>System logs</source>
        <extracomment>The syslog download description of the debug interface</extracomment>
        <translation>Järjestelmälokit</translation>
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
        <translation>Näytä</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1274"/>
        <source>Settings</source>
        <extracomment>The settings download section title of the debug interface</extracomment>
        <translation>Asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1284"/>
        <source>nymead settings</source>
        <extracomment>The nymead settings download description of the debug interface</extracomment>
        <translation>nymead-asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1384"/>
        <source>Rules settings</source>
        <extracomment>The rules settings download description of the debug interface</extracomment>
        <translation>Säännöt-asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1434"/>
        <source>Plugins settings</source>
        <extracomment>The plugins settings download description of the debug interface</extracomment>
        <translation>Lisäosien asetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1484"/>
        <source>Tag settings</source>
        <extracomment>The tag settings download description of the debug interface</extracomment>
        <translation>Tunnisteasetukset</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1533"/>
        <source>MQTT policies</source>
        <extracomment>The MQTT policies download description of the debug interface</extracomment>
        <translation>MQTT-käytännöt</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1583"/>
        <source>IO Connections</source>
        <extracomment>The MQTT policies download description of the debug interface</extracomment>
        <translation>IO-yhteydet</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1641"/>
        <source>This section allows you to perform different network connectivity tests in order to find out if the device where nymea is running has full network connectivity.</source>
        <extracomment>The network section description of the debug interface</extracomment>
        <translation>Tämä osio mahdollistaa erilaisten verkkoyhteystestien suorittamisen selvittääksesi, onko laitteella, jolla nymea toimii, täydet verkkoyhteydet.</translation>
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
        <translation>Tämä testi tekee neljä ping-yritystä nymea.io-palvelimeen.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1659"/>
        <source>Start ping test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation>Käynnistä ping-testi</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1675"/>
        <source>DNS lookup</source>
        <extracomment>The DNS lookup section of the debug interface</extracomment>
        <translation>DNS-haku</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1678"/>
        <source>This test makes a dynamic name server lookup for nymea.io.</source>
        <translation>Tämä testi tekee dynaamisen nimihakukyselyn nymea.io:lle.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1688"/>
        <source>Start DNS lookup test</source>
        <extracomment>The ping button text of the debug interface</extracomment>
        <translation>Käynnistä DNS-hakutesti</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1703"/>
        <source>Trace path</source>
        <extracomment>The trace section of the debug interface</extracomment>
        <translation>Jäljitysreitti</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1715"/>
        <source>Start trace path test</source>
        <extracomment>The trace path button text of the debug interface</extracomment>
        <translation>Käynnistä jäljitysreititesti</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1742"/>
        <source>Server live logs</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Palvelimen live-lokit</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1757"/>
        <source>Start logs</source>
        <extracomment>The connect button for the log stream of the debug interface</extracomment>
        <translation>Käynnistä lokit</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1796"/>
        <source>Logging filters</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Lokisuodattimet</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1835"/>
        <source>Logging filters plugins</source>
        <extracomment>The network section of the debug interface</extracomment>
        <translation>Lisäosien lokisuodattimet</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1874"/>
        <source>Released under the GNU GENERAL PUBLIC LICENSE Version 3.</source>
        <extracomment>The footer license note of the debug interface</extracomment>
        <translation>Julkaistu GNU GENERAL PUBLIC LICENSE Version 3 -lisenssin alaisuudessa.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1948"/>
        <source>Released under the GNU GENERAL PUBLIC LICENSE Version 2.</source>
        <translation>Julkaistu GNU GENERAL PUBLIC LICENSE, version 2 alla.</translation>
    </message>
    <message>
        <location filename="../libnymea-core/debugserverhandler.cpp" line="1918"/>
        <source>Error  %1</source>
        <extracomment>The HTTP error message of the debug interface. The %1 represents the error code ie.e 404</extracomment>
        <translation>Virhe %1</translation>
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
        <translation>Tiedostoa &quot;&amp;1&quot; ei löytynyt.</translation>
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
        <translation>Tiedostoa &quot;&amp;1&quot; ei voitu avata.</translation>
    </message>
</context>
</TS>
