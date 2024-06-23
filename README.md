# AsaApi.Plugins.ElementTransfer
ARK:SA ServerAPI Plugin that support Element Transfer between clusters

# Requirements
- ServerAPI [Download/Install](https://github.com/ArkServerApi/AsaApi)
- Ofcourse the ARK:SA Server
- Sqlite(automatic) or Mysql

# Features
- Transfer elements between clusters
- Required Upload/Download Terminal to use /upload /download commands
- Drops beacon, Transmitter, Obilisk and any other terminal
- commands can be change via config
- Able to spend points
- Permission groups

# Dependencies
- [ArkShop](https://gameservershub.com/forums/resources/ark-survival-ascended-arkshop-crossplay-supported.714/)
- [Permissions](https://gameservershub.com/forums/resources/ark-survival-ascended-permissions-crossplay-supported.713/)

# Blank Config
```
{
  "Mysql": {
    "MysqlDB": "",
    "MysqlHost": "",
    "MysqlPass": "",
    "MysqlPort": 3306,
    "MysqlUser": "",
    "UseMysql": false,
    "DbPathOverride": ""
  },
  "Points": {
    "Enable": true,
    "UploadCost": 10,
    "DownloadCost": 10
  },
  "Permissions": {
    "Enable": true,
    "Group": "Registered"
  },
  "Messages": {
    "Sender": "ElementTransfer",
    "UploadCMD": "/upload",
    "DownloadCMD": "/download",
    "UploadMsg": "{0} Elements has been uploaded",
    "DownloadMsg": "{0} Element succesfuly downloaded",
    "UploadErrorMsg": "Element Upload Error",
    "DownloadErrorMsg": "Element Download Error",
    "PermissionErrorMsg": "You dont have permission to use this command",
    "UploadNoElementMsg": "No Element in Inventory",
    "DownloadNoElementMsg": "No Element to download"
  }
}
```


# Commented Config
```
{
  "Mysql": {
    "MysqlDB": "", /* Database name */
    "MysqlHost": "", /* host or ip address */
    "MysqlPass": "", /* password */
    "MysqlPort": 3306, /* port 3306 default mostly */
    "MysqlUser": "", /* db username */
    "UseMysql": false, /* Mysql credentials is required if enabled */
    "DbPathOverride": "" /* sqlite database location */
  },
  "Permissions": {
    "Enable": true, /* Enabling permissions */
    "Group": "Registered" /* Permission group when linking */
  },
  "Messages": {
    "Sender": "ElementTransfer",
    "UploadCMD": "/upload",
    "DownloadCMD": "/download",
    "UploadMsg": "{0} Elements has been uploaded",
    "DownloadMsg": "{0} Element succesfuly downloaded",
    "UploadErrorMsg": "Element Upload Error",
    "DownloadErrorMsg": "Element Download Error",
    "PermissionErrorMsg": "You dont have permission to use this command",
    "UploadNoElementMsg": "No Element in Inventory",
    "DownloadNoElementMsg": "No Element to download"
  }
}
```

# RELOAD COMMANDS​
Both RCON and Console command: `ElementTransfer.Reload`

# Support
Feel free to join my Discord server [ArkTools](https://discord.gg/q8rPGprjEJ)

# Give Love!
[Buy me a coffee](https://www.buymeacoffee.com/ohmcodes)

[Paypal](https://www.paypal.com/donate/?business=8389QZ23QRDPE&no_recurring=0&item_name=Game+Server%2FTools+Community+Donations&currency_code=CAD)


.
