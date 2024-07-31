# Power Control Server

> [!CAUTION]
> **Power Control Server** is currently in development and may have bugs. Some of the features mentioned
> below, or the app for certain operating systems, may not be available in the current state.
>
> Please use the application with caution and report any issues via
> the [Issues](https://github.com/OffRange/PowerControlServer/issues) page.

**Power Control Server** is a lightweight application that enables remote management of your PC's power state via the **Power Control Android app**. By installing this server application, you can allow remote shutdown, reboot, log-out commands from your Android device.

### Prerequisites

For communication to the target PC, you must have installed the **Power Control** Android app. 

> [!NOTE]
> Currently the **Power Control** app is **only available for Android.**

You can find the Power Control app on a separate GitHub
repository: [Power Control](https://github.com/OffRange/PowerControl)

## Features

- **Remote Shutdown**: Allows your PC to be turned off remotely.
- **Remote Reboot**: Restarts your PC via a command from the Power Control app.
- **Remote Log-Out**: Logs out the current session on your PC.
- **Cross-Platform**: Available for Windows, Linux, and macOS.

## Installation

This section is added soon.


## Configuration

After installation, ensure that:

1. Your PC is on the same network as your Android device running the **Power Control** app.
2. The correct IP address is configured in the Power Control app.

### Wake-On LAN (Optional)

If you want to enable remote booting of your PC, ensure that Wake-On LAN is enabled in your BIOS/UEFI settings and your network adapter's settings in the operating system. After that add your network adapter's mac address to your registered device in the Android **Power Control** app.

## Usage

1. Start your PC, the **Power Control Server** automatically starts at every boot.
2. Open the **Power Control** app on your Android device.
3. Add a new device by entering your PC's IP address in the app.
4. Perform remote power operations like shutdown, reboot, or log-out directly from your Android device.

## Troubleshooting

- **Firewall Settings**: Ensure that your firewall allows incoming connections on the port used by the server (default is 7848).
- **Network Issues**: Make sure both your PC and Android device are connected to the same local network.
- **Application Logs**: Check the server logs for any error messages if the connection fails.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes. For major changes, please open an issue first to discuss what you would like to change.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
