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
  "General": {
    "NotifDisplayTime": 15.0,
    "NotifTextSize": 1.3,
    "ElementBPCheck": [
      "Blueprint'/Game/PrimalEarth/CoreBlueprints/Resources/PrimalItemResource_Element.PrimalItemResource_Element'",
      "Blueprint'/Game/PrimalEarth/CoreBlueprints/Resources/PrimalItemResource_ElementPowerNode.PrimalItemResource_ElementPowerNode'"
    ],
    "ElementBP": "Blueprint'/Game/PrimalEarth/CoreBlueprints/Resources/PrimalItemResource_Element.PrimalItemResource_Element'"
  },
  "Debug": {
    "ElementTransfer": false,
    "Permissions": false,
    "Points": false
  },
  "Commands": {
    "UploadCMD": "/upload",
    "DownloadCMD": "/download",
    "CheckUploadedCMD": "/checkupload",
    "CheckLimitCMD": "/checklimit"
  },
  "Messages": {
    "PermErrorMSG": "You don't have permission to use this command.",
    "PointsErrorMSG": "Not enough points.",
    "UploadErrorMSG": "Element Upload Error",
    "DownloadErrorMSG": "Element Download Error",
    "UploadMSG": "{0} Elements has been uploaded",
    "DownloadMsg": "{0} Elements succesfuly downloaded. {1} remaining",
    "UploadLimitMSG": "You have reached server maximum upload limit. {0}",
    "NoElementMSG": "No uploaded element",
    "NoAvailableSlotMSG": "No available slot.",
    "TerminalRequiredMSG": "Find nearest upload/download terminal",
    "SameMapNotAllowedMSG": "Downloading on the same map is not allowed",
    "CheckUploadedMSG": "You have {0}/{1} uploaded element.",
    "CheckLimitMSG": "Your current element upload limit {0}"
  },
  "DiscordBot": {
    "ChannelID": "",
    "Webhook": "",
    "BotImageURL": "https://cdn.discordapp.com/app-icons/1171816727340920922/da901b44e76d1d880c876dc007437a3f.png",
    "BotUsername": "ElementTransfer",
    "DiscordEmbedTemplate": "",
    "Messages": {
      "UploadMSG": "Player {0} uploaded element {1} Map: {2}",
      "DownloadMSG": "Player {0} downloaded element {1} on Map {2}",
      "CheckUploadedMSG": "{0} has {1}/{2} uploaded element",
      "CheckLimitMSG": "{0} check element upload limit {1}"
    }
  },
  "PluginDBSettings": {
    "UseMySQL": false,
    "Host": "",
    "User": "",
    "Password": "",
    "Database": "",
    "TableName": "ElementTransfer",
    "Port": 3306,
    "MysqlSSLMode": -1,
    "MysqlTLSVersion": "",
    "SQLiteDatabasePath": ""
  },
  "PermissionsDBSettings": {
    "Enabled": false,
    "UseMySQL": false,
    "Host": "",
    "User": "",
    "Password": "",
    "Database": "",
    "TableName": "Players",
    "UniqueIDField": "EOS_Id",
    "PermissionGroupField": "PermissionGroups",
    "Port": 3306,
    "MysqlSSLMode": -1,
    "MysqlTLSVersion": "",
    "SQLiteDatabasePath": ""
  },
  "PointsDBSettings": {
    "Enabled": false,
    "UseMySQL": false,
    "Host": "",
    "User": "",
    "Password": "",
    "Database": "",
    "TableName": "ArkShopPlayers",
    "UniqueIDField": "EosId",
    "PointsField": "Points",
    "TotalSpentField": "TotalSpent",
    "Port": 3306,
    "MysqlSSLMode": -1,
    "MysqlTLSVersion": "",
    "SQLiteDatabasePath": ""
  },
  "PermissionGroups": {
    "Default": {
      "Priority": 99
    },
    "Admins": {
      "Priority": 1,
      "Commands": {
        "UploadCMD": {
          "Enabled": true,
          "Cost": 50,
          "NotifDiscord": true,
          "AllowUploadAnywhere": true,
          "LimitUploadCount": -1,
          "TriggerInterval": 0
        },
        "DownloadCMD": {
          "Enabled": true,
          "Cost": 0,
          "NotifDiscord": true,
          "AllowDownloadAnywhere": true,
          "AllowDownloadSameMap": true,
          "TriggerInterval": 0
        },
        "CheckUploadedCMD": {
          "Enabled": true,
          "Cost": 0,
          "NotifDiscord": true,
          "TriggerInterval": 0
        },
        "CheckLimitCMD": {
          "Enabled": true,
          "Cost": 0,
          "NotifDiscord": true,
          "TriggerInterval": 0
        }
      }
    }
  }
}
```


# Commented Config
```
{
  "General": {
    "NotifDisplayTime": 15.0,
    "NotifTextSize": 1.3,
    "ElementBPCheck": [
      "Blueprint'/Game/PrimalEarth/CoreBlueprints/Resources/PrimalItemResource_Element.PrimalItemResource_Element'",
      "Blueprint'/Game/PrimalEarth/CoreBlueprints/Resources/PrimalItemResource_ElementPowerNode.PrimalItemResource_ElementPowerNode'"
        // add more data here if you have modded element
    ],
    "ElementBP": "Blueprint'/Game/PrimalEarth/CoreBlueprints/Resources/PrimalItemResource_Element.PrimalItemResource_Element'"
      // set your desired downloaded element default: vanilla
  },
  "Debug": {
    "ElementTransfer": false,
    "Permissions": false,
    "Points": false
  },
  "Commands": {
    "UploadCMD": "/upload", // will allow player to set parameters eg: /upload 50
    "DownloadCMD": "/download", // will allow player to set parameters eg: /upload 50
    "CheckUploadedCMD": "/checkupload",
    "CheckLimitCMD": "/checklimit"
  },
  "Messages": {
    "PermErrorMSG": "You don't have permission to use this command.",
    "PointsErrorMSG": "Not enough points.",
    "UploadErrorMSG": "Element Upload Error",
    "DownloadErrorMSG": "Element Download Error",
    "UploadMSG": "{0} Elements has been uploaded",
    "DownloadMsg": "{0} Elements succesfuly downloaded. {1} remaining",
    "UploadLimitMSG": "You have reached server maximum upload limit. {0}",
    "NoElementMSG": "No uploaded element",
    "NoAvailableSlotMSG": "No available slot.",
    "TerminalRequiredMSG": "Find nearest upload/download terminal",
    "SameMapNotAllowedMSG": "Downloading on the same map is not allowed",
    "CheckUploadedMSG": "You have {0}/{1} uploaded element.",
    "CheckLimitMSG": "Your current element upload limit {0}"
  },
  "DiscordBot": {
    "ChannelID": "",
    "Webhook": "",
    "BotImageURL": "https://cdn.discordapp.com/app-icons/1171816727340920922/da901b44e76d1d880c876dc007437a3f.png",
    "BotUsername": "ElementTransfer",
    "DiscordEmbedTemplate": "",
    "Messages": {
      "UploadMSG": "Player {0} uploaded element {1} Map: {2}",
      "DownloadMSG": "Player {0} downloaded element {1} on Map {2}",
      "CheckUploadedMSG": "{0} has {1}/{2} uploaded element",
      "CheckLimitMSG": "{0} check element upload limit {1}"
    }
  },
  "PluginDBSettings": {
    "UseMySQL": false,
    "Host": "",
    "User": "",
    "Password": "",
    "Database": "",
    "TableName": "ElementTransfer",
    "Port": 3306,
    "MysqlSSLMode": -1,
    "MysqlTLSVersion": "",
    "SQLiteDatabasePath": ""
  },
  "PermissionsDBSettings": {
    "Enabled": false,
    "UseMySQL": false,
    "Host": "",
    "User": "",
    "Password": "",
    "Database": "",
    "TableName": "Players",
    "UniqueIDField": "EOS_Id",
    "PermissionGroupField": "PermissionGroups",
    "Port": 3306,
    "MysqlSSLMode": -1,
    "MysqlTLSVersion": "",
    "SQLiteDatabasePath": ""
  },
  "PointsDBSettings": {
    "Enabled": false,
    "UseMySQL": false,
    "Host": "",
    "User": "",
    "Password": "",
    "Database": "",
    "TableName": "ArkShopPlayers",
    "UniqueIDField": "EosId",
    "PointsField": "Points",
    "TotalSpentField": "TotalSpent",
    "Port": 3306,
    "MysqlSSLMode": -1,
    "MysqlTLSVersion": "",
    "SQLiteDatabasePath": ""
  },
  "PermissionGroups": {
    "Default": {
      "Priority": 99
    },
    "Admins": {
      "Priority": 1,
      "Commands": {
        "UploadCMD": {
          "Enabled": true,
          "Cost": 50,
          "NotifDiscord": true,
          "AllowUploadAnywhere": true,
          "LimitUploadCount": -1, // -1 for no limit. 0 and more will set limit
          "TriggerInterval": 0 // cooldown
        },
        "DownloadCMD": {
          "Enabled": true,
          "Cost": 0,
          "NotifDiscord": true,
          "AllowDownloadAnywhere": true,
          "AllowDownloadSameMap": true,
          "TriggerInterval": 0 // cooldown
        },
        "CheckUploadedCMD": {
          "Enabled": true,
          "Cost": 0,
          "NotifDiscord": true,
          "TriggerInterval": 0 // cooldown
        },
        "CheckLimitCMD": {
          "Enabled": true,
          "Cost": 0,
          "NotifDiscord": true,
          "TriggerInterval": 0 // cooldown
        }
      }
    }
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
