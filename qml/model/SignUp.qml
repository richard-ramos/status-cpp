pragma Singleton

import QtQuick 2.13
import im.status.desktop 1.0

QtObject {
    function setupAccount(account, password) {

      console.log(typeof account)
        
      Status.multiAccountStoreDerivedAccounts(account, password);

      
    }

   

    /*

    proc setupAccount*(fleetConfig: FleetConfig, account: GeneratedAccount, password: string): types.Account =
    try:
        let storeDerivedResult = storeDerivedAccounts(account, password)
        let accountData = getAccountData(account)
        let installationId = $genUUID()
        var settingsJSON = getAccountSettings(account, constants.DEFAULT_NETWORKS, installationId)
        var nodeConfig = getNodeConfig(fleetConfig, installationId)

        result = saveAccountAndLogin(account, $accountData, password, $nodeConfig, $settingsJSON)


proc getNodeConfig*(fleetConfig: FleetConfig, installationId: string, networkConfig: JsonNode, fleet: Fleet = Fleet.PROD): JsonNode =
  let upstreamUrl = networkConfig["config"]["UpstreamConfig"]["URL"]
  var newDataDir = networkConfig["config"]["DataDir"].getStr
  newDataDir.removeSuffix("_rpc")

  result = constants.NODE_CONFIG.copy()
  result["ClusterConfig"]["Fleet"] = newJString($fleet)
  result["ClusterConfig"]["BootNodes"] = %* fleetConfig.getNodes(fleet, FleetNodes.Bootnodes)
  result["ClusterConfig"]["TrustedMailServers"] = %* fleetConfig.getNodes(fleet, FleetNodes.Mailservers)
  result["ClusterConfig"]["StaticNodes"] = %* fleetConfig.getNodes(fleet, FleetNodes.Whisper)
  result["ClusterConfig"]["RendezvousNodes"] = %* fleetConfig.getNodes(fleet, FleetNodes.Rendezvous)
  result["Rendezvous"] = newJBool(fleetConfig.getNodes(fleet, FleetNodes.Rendezvous).len > 0)
  result["NetworkId"] = networkConfig["config"]["NetworkId"]
  result["DataDir"] = newDataDir.newJString()
  result["UpstreamConfig"]["Enabled"] = networkConfig["config"]["UpstreamConfig"]["Enabled"]
  result["UpstreamConfig"]["URL"] = upstreamUrl
  result["ShhextConfig"]["InstallationID"] = newJString(installationId)
  result["ListenAddr"] = if existsEnv("STATUS_PORT"): newJString("0.0.0.0:" & $getEnv("STATUS_PORT")) else: newJString("0.0.0.0:30305")
  


proc getAccountSettings*(account: GeneratedAccount, defaultNetworks: JsonNode, installationId: string): JsonNode =
  result = %* {
    "key-uid": account.keyUid,
    "mnemonic": account.mnemonic,
    "public-key": account.derived.whisper.publicKey,
    "name": account.name,
    "address": account.address,
    "eip1581-address": account.derived.eip1581.address,
    "dapps-address": account.derived.defaultWallet.address,
    "wallet-root-address": account.derived.walletRoot.address,
    "preview-privacy?": true,
    "signing-phrase": generateSigningPhrase(3),
    "log-level": "INFO",
    "latest-derived-path": 0,
    "networks/networks": defaultNetworks,
    "currency": "usd",
    "identicon": account.identicon,
    "waku-enabled": true,
    "wallet/visible-tokens": {
      "mainnet": ["SNT"]
    },
    "appearance": 0,
    "networks/current-network": constants.DEFAULT_NETWORK_NAME,
    "installation-id": installationId
  }

proc getAccountData*(account: GeneratedAccount): JsonNode =
  result = %* {
    "name": account.name,
    "address": account.address,
    "identicon": account.identicon,
    "key-uid": account.keyUid,
    "keycard-pairing": nil
  }




proc saveAccountAndLogin*(
  account: GeneratedAccount,
  accountData: string,
  password: string,
  configJSON: string,
  settingsJSON: string): types.Account =
  let hashedPassword = hashPassword(password)
  let subaccountData = %* [
    {
      "public-key": account.derived.defaultWallet.publicKey,
      "address": account.derived.defaultWallet.address,
      "color": "#4360df",
      "wallet": true,
      "path": constants.PATH_DEFAULT_WALLET,
      "name": "Status account"
    },
    {
      "public-key": account.derived.whisper.publicKey,
      "address": account.derived.whisper.address,
      "name": account.name,
      "identicon": account.identicon,
      "path": constants.PATH_WHISPER,
      "chat": true
    }
  ]

  var savedResult = $nim_status.saveAccountAndLogin(accountData, hashedPassword, settingsJSON, configJSON, $subaccountData)
  let parsedSavedResult = savedResult.parseJson
  let error = parsedSavedResult["error"].getStr

  if error == "":
    debug "Account saved succesfully"
    result = account.toAccount
    return

  raise newException(StatusGoException, "Error saving account and logging in: " & error)


    */
}