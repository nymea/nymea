# nymea - open source IoT edge server
--------------------------------------------
<p align="center">
  <a  href="https://nymea.io">
    <img src="https://nymea.io/downloads/img/nymea-logo.svg" width=300>
  </a>
</p>

nymea (/[n'aiːmea:]/ - is an open source IoT edge server. The plug-in based architecture allows to integrate protocols and APIs. With the build-in rule engine you are able to interconnect devices or services available in the system and create individual scenes and behaviours for your environment.

## Quick start

### Install nymea on a Raspberry Pi

<p align="center">
<img src="https://nymea.io/downloads/img/nymea-pi.svg" width=300 >
</p>

We have created an image for your Raspberry Pi that comes with an array of plugins for different smart devices!

Use the [Raspberry Pi imager](https://www.raspberrypi.com/software/) and select nymea as operating system. You'll have the
choice between a headless nymea:core setup or a kiosk image that contains nymea:core and nymea:app for Raspberry Pis with touch screen.

Alternatively, the image can be downloaded and flashed manually from [here](https://downloads.nymea.io/images/raspberrypi/). We recommend
the latest Raspberry Pi OS Bookworm [core](https://downloads.nymea.io/images/raspberrypi/nymea-core-image-raspios-bookworm-latest.zip) or
[kiosk](https://downloads.nymea.io//images/raspberrypi/nymea-kiosk-image-raspios-bookworm-latest.zip) image.

### Get nymea:app here:

<table align="middle">
  <tr>
    <td> 
      <p>
        <a href="https://apps.apple.com/us/app/nymea-app/id1400810250">
          <img border="0" align="middle" alt="iOS Badge" src="https://nymea.io/downloads/img/app-store/appstore.png" width=200>
     </p>
    </td>
    <td> 
      <p>
         <a href="https://play.google.com/store/apps/details?id=io.guh.nymeaapp&hl=en&pcampaignid=MKT-Other-global-all-co-prtnr-py-PartBadge-Mar2515-1">
         <img border="0" align="middle" alt="Android Badge" src="https://play.google.com/intl/en_us/badges/static/images/badges/en_badge_web_generic.png" width=250>
     </p>
    </td>
    <td> 
      <p>
        <a href="https://open-store.io/app/io.guh.nymeaapp">
        <img border="0" align="middle" alt="Openstore Badge" src="https://open-store.io/badges/en_US.png" width=200>
      </p>
    </td>
    <td> 
      <p>
         <a href="https://apps.apple.com/us/app/nymea-app/id1488785734">
         <img border="0" align="middle" alt="macOS Badge" src="https://nymea.io/downloads/img/app-store/macos.png" width=200">
      </p>
    </td>
    <td> 
      <p>
        <a href="https://snapcraft.io/nymea-app">
        <img border="0" align="middle" alt="Snap Badge" src="https://nymea.io/downloads/img/app-store/snap-store.png" width=200>
      </p>
    </td>
    <td> 
      <p>
        <a href="https://downloads.nymea.io/nymea-app/windows/latest">
        <img border="0" align="middle" alt="Windows Badge" src="https://nymea.io/downloads/img/app-store/windows.svg" width=200>
      </p>
    </td>
  </tr>
</table>

Manual download files of nymea:app can be found [here](https://downloads.nymea.io/nymea-app/).

A detailed description how to install and getting started with *nymea* can be found in the [nymea | user documentation](https://nymea.io/documentation/users/installation/getting-started).


## Getting help

If you want to present your project or want to share your newest developments you can share it in
[Our Forum](https://forum.nymea.io)

If you are facing any troubles, don't hesitate to reach out for us or the community members, we will be pleased to help you:
Chat with us on [Telegram](http://t.me/nymeacommunity) or [Discord](https://discord.gg/tX9YCpD)

## Developing with nymea

A detailed documentation on how to develop with *nymea* is available on the [nymea | developer documentation](https://nymea.io/documentation/developers/).


## Network discovery

When starting nymead as user without root privileges, the network device discovery will not available due to missing raw socket permission. 
If you still want to make use of this feature, the binary capabilities need to be adjusted.

    sudo setcap cap_net_admin,cap_net_raw=eip /usr/bin/nymead

This will allow nymead to create raw sockets for ARP and ICMP network discovery tools even when nymead gets started as user without root privileges. 

## License
--------------------------------------------
> nymea is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.
