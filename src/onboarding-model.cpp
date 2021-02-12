#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QUuid>
#include <QtConcurrent>

#include <algorithm>
#include <array>

#include "constants.hpp"
#include "libstatus.h"
#include "onboarding-model.hpp"
#include "signing-phrases.hpp"
#include "utils.hpp"

OnboardingModel::OnboardingModel(QObject* parent)
	: QAbstractListModel(parent)
{ }

GeneratedAccount jsonObjectToAccount(const QJsonObject obj)
{
	GeneratedAccount acc = {};
	acc.id = obj["id"].toString();
	acc.address = obj["address"].toString();
	acc.keyUid = obj["keyUid"].toString();
	acc.mnemonic = obj["mnemonic"].toString(); // TODO: clear from memory?
	acc.publicKey = obj["publicKey"].toString();
	return acc;
}

void setDerivedKeys(GeneratedAccount& acc, const QJsonObject obj)
{
	acc.derivedKeys = {{Constants::PathWalletRoot,
						{obj[Constants::PathWalletRoot]["publicKey"].toString(),
						 obj[Constants::PathWalletRoot]["address"].toString()}},
					   {Constants::PathEip1581,
						{obj[Constants::PathEip1581]["publicKey"].toString(),
						 obj[Constants::PathEip1581]["address"].toString()}},
					   {Constants::PathWhisper,
						{obj[Constants::PathWhisper]["publicKey"].toString(),
						 obj[Constants::PathWhisper]["address"].toString()}},
					   {Constants::PathDefaultWallet,
						{obj[Constants::PathDefaultWallet]["publicKey"].toString(),
						 obj[Constants::PathDefaultWallet]["address"].toString()}}};
}

QVector<GeneratedAccount> multiAccountGenerateAndDeriveAddresses()
{
	QJsonObject obj{{"n", 5},
					{"mnemonicPhraseLength", 12},
					{"bip32Passphrase", ""},
					{"paths",
					 QJsonArray{Constants::PathWalletRoot,
								Constants::PathEip1581,
								Constants::PathWhisper,
								Constants::PathDefaultWallet}}

	};
	const char* result = MultiAccountGenerateAndDeriveAddresses(
		Utils::jsonToStr(obj).toUtf8().data()); // TODO: clear from memory
	QJsonArray multiAccounts = QJsonDocument::fromJson(result).array();

	QVector<GeneratedAccount> vector;
	foreach(const QJsonValue& value, multiAccounts)
	{
		GeneratedAccount acc = jsonObjectToAccount(value.toObject());
		setDerivedKeys(acc, value.toObject()["derived"].toObject());
		vector.append(acc);
	}
	return vector;
}

void OnboardingModel::populate()
{
	beginResetModel();
	mData.clear();
	mData << multiAccountGenerateAndDeriveAddresses();
	endResetModel();
}

// TODO: destructor. Clear

QHash<int, QByteArray> OnboardingModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Id] = "id";
	roles[PublicKey] = "publicKey";
	roles[Name] = "name";
	roles[Image] = "image";
	return roles;
}

int OnboardingModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return mData.size();
}

QVariant OnboardingModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	switch(role)
	{
	case Id:
		return QVariant(mData[index.row()].keyUid);
	case PublicKey:
		return QVariant(mData[index.row()].derivedKeys.at(Constants::PathWhisper).publicKey);
	case Image:
		return QVariant(Utils::generateIdenticon(
			mData[index.row()].derivedKeys.at(Constants::PathWhisper).publicKey));
	case Name:
		return QVariant(Utils::generateAlias(
			mData[index.row()].derivedKeys.at(Constants::PathWhisper).publicKey));
	}

	return QVariant();
}

QString OnboardingModel::getAccountId(int index)
{
	return mData[index].keyUid;
}

QVariantMap OnboardingModel::get(int row) const
{
	QHash<int, QByteArray> names = roleNames();
	QHashIterator<int, QByteArray> i(names);
	QVariantMap res;
	QModelIndex idx = index(row, 0);
	while(i.hasNext())
	{
		i.next();
		QVariant data = idx.data(i.key());
		res[i.value()] = data;
	}
	return res;
}

void storeDerivedAccount(QString& accountId, QString& hashedPassword)
{
	QJsonObject obj{{"accountID", accountId},
					{"paths",
					 QJsonArray{Constants::PathWalletRoot,
								Constants::PathEip1581,
								Constants::PathWhisper,
								Constants::PathDefaultWallet}},
					{"password", hashedPassword}};

	MultiAccountStoreDerivedAccounts(Utils::jsonToStr(obj).toUtf8().data());

	// TODO: clear obj
}

QJsonObject getAccountData(GeneratedAccount* account)
{
	return QJsonObject{
		{"name", Utils::generateAlias(account->derivedKeys[Constants::PathWhisper].publicKey)},
		{"address", account->address},
		{"photo-path",
		 Utils::generateIdenticon(account->derivedKeys[Constants::PathWhisper].publicKey)},
		{"identicon",
		 Utils::generateIdenticon(account->derivedKeys[Constants::PathWhisper].publicKey)},
		{"key-uid", account->keyUid},
		{"keycard-pairing", QJsonValue()}};
}

QString generateSigningPhrase(int count)
{
	QStringList words;
	for(int i = 0; i < count; i++)
	{
		words.append(
			phrases[QRandomGenerator::global()->bounded(static_cast<int>(phrases.size()))]);
	}
	return words.join(" ");
}

QJsonObject getAccountSettings(GeneratedAccount* account, QString installationId)
{
	// TODO: this does not work on android
	//QFile defaultNetworks("../resources/default-networks.json");
	//defaultNetworks.open(QIODevice::ReadOnly);

	// QString defaultNetworksContent = defaultNetworks.readAll();

	QString defaultNetworksContent = QString("[{\"id\":\"testnet_rpc\",\"etherscan-link\":\"https://ropsten.etherscan.io/address/\",\"name\":\"Ropsten with upstream RPC\",\"config\":{\"NetworkId\":3,\"DataDir\":\"/ethereum/testnet_rpc\",\"UpstreamConfig\":{\"Enabled\":true,\"URL\":\"https://ropsten.infura.io/v3/%INFURA_KEY%\"}}},{\"id\":\"rinkeby_rpc\",\"etherscan-link\":\"https://rinkeby.etherscan.io/address/\",\"name\":\"Rinkeby with upstream RPC\",\"config\":{\"NetworkId\":4,\"DataDir\":\"/ethereum/rinkeby_rpc\",\"UpstreamConfig\":{\"Enabled\":true,\"URL\":\"https://rinkeby.infura.io/v3/%INFURA_KEY%\"}}},{\"id\":\"goerli_rpc\",\"etherscan-link\":\"https://goerli.etherscan.io/address/\",\"name\":\"Goerli with upstream RPC\",\"config\":{\"NetworkId\":5,\"DataDir\":\"/ethereum/goerli_rpc\",\"UpstreamConfig\":{\"Enabled\":true,\"URL\":\"https://goerli.blockscout.com/\"}}},{\"id\":\"mainnet_rpc\",\"etherscan-link\":\"https://etherscan.io/address/\",\"name\":\"Mainnet with upstream RPC\",\"config\":{\"NetworkId\":1,\"DataDir\":\"/ethereum/mainnet_rpc\",\"UpstreamConfig\":{\"Enabled\":true,\"URL\":\"https://mainnet.infura.io/v3/%INFURA_KEY%\"}}},{\"id\":\"xdai_rpc\",\"name\":\"xDai Chain\",\"config\":{\"NetworkId\":100,\"DataDir\":\"/ethereum/xdai_rpc\",\"UpstreamConfig\":{\"Enabled\":true,\"URL\":\"https://dai.poa.network\"}}},{\"id\":\"poa_rpc\",\"name\":\"POA Network\",\"config\":{\"NetworkId\":99,\"DataDir\":\"/ethereum/poa_rpc\",\"UpstreamConfig\":{\"Enabled\":true,\"URL\":\"https://core.poa.network\"}}}]");

	defaultNetworksContent = defaultNetworksContent.replace("%INFURA_KEY%", INFURA_KEY);

	QJsonArray defaultNetworksJson =
		QJsonDocument::fromJson(defaultNetworksContent.toUtf8()).array();

	return QJsonObject{
		{"key-uid", account->keyUid},
		{"mnemonic", account->mnemonic},
		{"public-key", account->derivedKeys[Constants::PathWhisper].publicKey},
		{"name", Utils::generateAlias(account->publicKey)},
		{"address", account->address},
		{"eip1581-address", account->derivedKeys[Constants::PathEip1581].address},
		{"dapps-address", account->derivedKeys[Constants::PathDefaultWallet].address},
		{"wallet-root-address", account->derivedKeys[Constants::PathDefaultWallet].address},
		{"preview-privacy?", true},
		{"signing-phrase", generateSigningPhrase(3)},
		{"log-level", "INFO"},
		{"latest-derived-path", 0},
		{"networks/networks", defaultNetworksJson},
		{"currency", "usd"},
		{"identicon",
		 Utils::generateIdenticon(account->derivedKeys[Constants::PathWhisper].publicKey)},
		{"waku-enabled", true},
		{"wallet/visible-tokens", {{"mainnet", QJsonArray{"SNT"}}}},
		{"appearance", 0},
		{"networks/current-network", Constants::DefaultNetworkName},
		{"installation-id", installationId}};

	// TODO: clear mnemonic?
}

QJsonObject getNodeConfig(QString installationId)
{
	// TODO: this does not work on android.
	//QFile nodeConfig("../resources/node-config.json");
	//nodeConfig.open(QIODevice::ReadOnly);


	//QString nodeConfigContent = nodeConfig.readAll();
	QString nodeConfigContent = QString("{\"BrowsersConfig\":{\"Enabled\":true},\"ClusterConfig\":{\"BootNodes\":[],\"Enabled\":true,\"Fleet\":\"eth.prod\",\"RendezvousNodes\":[],\"StaticNodes\":[],\"TrustedMailServers\":[]},\"DataDir\":\"./ethereum/mainnet\",\"EnableNTPSync\":true,\"KeyStoreDir\":\"./keystore\",\"ListenAddr\":\"0.0.0.0:30306\",\"LogEnabled\":true,\"LogFile\":\"./geth.log\",\"LogLevel\":\"INFO\",\"MailserversConfig\":{\"Enabled\":true},\"Name\":\"StatusIM\",\"NetworkId\":1,\"NoDiscovery\":false,\"PermissionsConfig\":{\"Enabled\":true},\"Rendezvous\":true,\"RequireTopics\":{\"whisper\":{\"Max\":2,\"Min\":2}},\"ShhextConfig\":{\"BackupDisabledDataDir\":\"./\",\"DataSyncEnabled\":true,\"InstallationID\":\"%INSTALLATIONID%\",\"MailServerConfirmations\":true,\"MaxMessageDeliveryAttempts\":6,\"PFSEnabled\":true,\"VerifyENSContractAddress\":\"0x00000000000C2E074eC69A0dFb2997BA6C7d2e1e\",\"VerifyENSURL\":\"https://mainnet.infura.io/v3/%INFURA_KEY%\",\"VerifyTransactionChainID\":1,\"VerifyTransactionURL\":\"https://mainnet.infura.io/v3/%INFURA_KEY%\"},\"StatusAccountsConfig\":{\"Enabled\":true},\"UpstreamConfig\":{\"Enabled\":true,\"URL\":\"https://mainnet.infura.io/v3/%INFURA_KEY%\"},\"WakuConfig\":{\"BloomFilterMode\":null,\"Enabled\":true,\"LightClient\":true,\"MinimumPoW\":0.001},\"WalletConfig\":{\"Enabled\":true}}");


	nodeConfigContent = nodeConfigContent.replace("%INSTALLATIONID%", installationId);
	nodeConfigContent = nodeConfigContent.replace("%INFURA_KEY%", INFURA_KEY);

	QJsonObject nodeConfigJson = QJsonDocument::fromJson(nodeConfigContent.toUtf8()).object();


	// TODO: this does not work on android
	//QFile fleets("../resources/fleets.json");
	//fleets.open(QIODevice::ReadOnly);

	QString fleets = QString("{\"fleets\":{\"eth.prod\":{\"boot\":{\"boot-01.ac-cn-hongkong-c.eth.prod\":\"enode://6e6554fb3034b211398fcd0f0082cbb6bd13619e1a7e76ba66e1809aaa0c5f1ac53c9ae79cf2fd4a7bacb10d12010899b370c75fed19b991d9c0cdd02891abad@47.75.99.169:443\",\"boot-01.do-ams3.eth.prod\":\"enode://436cc6f674928fdc9a9f7990f2944002b685d1c37f025c1be425185b5b1f0900feaf1ccc2a6130268f9901be4a7d252f37302c8335a2c1a62736e9232691cc3a@178.128.138.128:443\",\"boot-01.gc-us-central1-a.eth.prod\":\"enode://32ff6d88760b0947a3dee54ceff4d8d7f0b4c023c6dad34568615fcae89e26cc2753f28f12485a4116c977be937a72665116596265aa0736b53d46b27446296a@34.70.75.208:443\",\"boot-02.ac-cn-hongkong-c.eth.prod\":\"enode://23d0740b11919358625d79d4cac7d50a34d79e9c69e16831c5c70573757a1f5d7d884510bc595d7ee4da3c1508adf87bbc9e9260d804ef03f8c1e37f2fb2fc69@47.52.106.107:443\",\"boot-02.do-ams3.eth.prod\":\"enode://5395aab7833f1ecb671b59bf0521cf20224fe8162fc3d2675de4ee4d5636a75ec32d13268fc184df8d1ddfa803943906882da62a4df42d4fccf6d17808156a87@178.128.140.188:443\",\"boot-02.gc-us-central1-a.eth.prod\":\"enode://5405c509df683c962e7c9470b251bb679dd6978f82d5b469f1f6c64d11d50fbd5dd9f7801c6ad51f3b20a5f6c7ffe248cc9ab223f8bcbaeaf14bb1c0ef295fd0@35.223.215.156:443\"},\"mail\":{\"mail-01.ac-cn-hongkong-c.eth.prod\":\"enode://606ae04a71e5db868a722c77a21c8244ae38f1bd6e81687cc6cfe88a3063fa1c245692232f64f45bd5408fed5133eab8ed78049332b04f9c110eac7f71c1b429@47.75.247.214:443\",\"mail-01.do-ams3.eth.prod\":\"enode://c42f368a23fa98ee546fd247220759062323249ef657d26d357a777443aec04db1b29a3a22ef3e7c548e18493ddaf51a31b0aed6079bd6ebe5ae838fcfaf3a49@178.128.142.54:443\",\"mail-01.gc-us-central1-a.eth.prod\":\"enode://ee2b53b0ace9692167a410514bca3024695dbf0e1a68e1dff9716da620efb195f04a4b9e873fb9b74ac84de801106c465b8e2b6c4f0d93b8749d1578bfcaf03e@104.197.238.144:443\",\"mail-02.ac-cn-hongkong-c.eth.prod\":\"enode://2c8de3cbb27a3d30cbb5b3e003bc722b126f5aef82e2052aaef032ca94e0c7ad219e533ba88c70585ebd802de206693255335b100307645ab5170e88620d2a81@47.244.221.14:443\",\"mail-02.do-ams3.eth.prod\":\"enode://7aa648d6e855950b2e3d3bf220c496e0cae4adfddef3e1e6062e6b177aec93bc6cdcf1282cb40d1656932ebfdd565729da440368d7c4da7dbd4d004b1ac02bf8@178.128.142.26:443\",\"mail-02.gc-us-central1-a.eth.prod\":\"enode://30211cbd81c25f07b03a0196d56e6ce4604bb13db773ff1c0ea2253547fafd6c06eae6ad3533e2ba39d59564cfbdbb5e2ce7c137a5ebb85e99dcfc7a75f99f55@23.236.58.92:443\",\"mail-03.ac-cn-hongkong-c.eth.prod\":\"enode://e85f1d4209f2f99da801af18db8716e584a28ad0bdc47fbdcd8f26af74dbd97fc279144680553ec7cd9092afe683ddea1e0f9fc571ebcb4b1d857c03a088853d@47.244.129.82:443\",\"mail-03.do-ams3.eth.prod\":\"enode://8a64b3c349a2e0ef4a32ea49609ed6eb3364be1110253c20adc17a3cebbc39a219e5d3e13b151c0eee5d8e0f9a8ba2cd026014e67b41a4ab7d1d5dd67ca27427@178.128.142.94:443\",\"mail-03.gc-us-central1-a.eth.prod\":\"enode://44160e22e8b42bd32a06c1532165fa9e096eebedd7fa6d6e5f8bbef0440bc4a4591fe3651be68193a7ec029021cdb496cfe1d7f9f1dc69eb99226e6f39a7a5d4@35.225.221.245:443\"},\"rendezvous\":{\"boot-01.ac-cn-hongkong-c.eth.prod\":\"/ip4/47.75.99.169/tcp/30703/ethv4/16Uiu2HAmV8Hq9e3zm9TMVP4zrVHo3BjqW5D6bDVV6VQntQd687e4\",\"boot-01.do-ams3.eth.prod\":\"/ip4/178.128.138.128/tcp/30703/ethv4/16Uiu2HAmRHPzF3rQg55PgYPcQkyvPVH9n2hWsYPhUJBZ6kVjJgdV\",\"boot-01.gc-us-central1-a.eth.prod\":\"/ip4/34.70.75.208/tcp/30703/ethv4/16Uiu2HAm6ZsERLx2BwVD2UM9SVPnnMU6NBycG8XPtu8qKys5awsU\",\"boot-02.ac-cn-hongkong-c.eth.prod\":\"/ip4/47.52.106.107/tcp/30703/ethv4/16Uiu2HAmEHiptiDDd9gqNY8oQqo8hHUWMHJzfwt5aLRdD6W2zcXR\",\"boot-02.do-ams3.eth.prod\":\"/ip4/178.128.140.188/tcp/30703/ethv4/16Uiu2HAmLqTXuY4Sb6G28HNooaFUXUKzpzKXCcgyJxgaEE2i5vnf\",\"boot-02.gc-us-central1-a.eth.prod\":\"/ip4/35.223.215.156/tcp/30703/ethv4/16Uiu2HAmQEUFE2YaJohavWtHxPTEFv3sEGJtDqvtGEv78DFoEWQF\"},\"whisper\":{\"node-01.ac-cn-hongkong-c.eth.prod\":\"enode://b957e51f41e4abab8382e1ea7229e88c6e18f34672694c6eae389eac22dab8655622bbd4a08192c321416b9becffaab11c8e2b7a5d0813b922aa128b82990dab@47.75.222.178:443\",\"node-01.do-ams3.eth.prod\":\"enode://66ba15600cda86009689354c3a77bdf1a97f4f4fb3ab50ffe34dbc904fac561040496828397be18d9744c75881ffc6ac53729ddbd2cdbdadc5f45c400e2622f7@178.128.141.87:443\",\"node-01.gc-us-central1-a.eth.prod\":\"enode://182ed5d658d1a1a4382c9e9f7c9e5d8d9fec9db4c71ae346b9e23e1a589116aeffb3342299bdd00e0ab98dbf804f7b2d8ae564ed18da9f45650b444aed79d509@34.68.132.118:443\",\"node-02.ac-cn-hongkong-c.eth.prod\":\"enode://8bebe73ddf7cf09e77602c7d04c93a73f455b51f24ae0d572917a4792f1dec0bb4c562759b8830cc3615a658d38c1a4a38597a1d7ae3ba35111479fc42d65dec@47.75.85.212:443\",\"node-02.do-ams3.eth.prod\":\"enode://4ea35352702027984a13274f241a56a47854a7fd4b3ba674a596cff917d3c825506431cf149f9f2312a293bb7c2b1cca55db742027090916d01529fe0729643b@134.209.136.79:443\",\"node-02.gc-us-central1-a.eth.prod\":\"enode://fbeddac99d396b91d59f2c63a3cb5fc7e0f8a9f7ce6fe5f2eed5e787a0154161b7173a6a73124a4275ef338b8966dc70a611e9ae2192f0f2340395661fad81c0@34.67.230.193:443\",\"node-03.ac-cn-hongkong-c.eth.prod\":\"enode://ac3948b2c0786ada7d17b80cf869cf59b1909ea3accd45944aae35bf864cc069126da8b82dfef4ddf23f1d6d6b44b1565c4cf81c8b98022253c6aea1a89d3ce2@47.75.88.12:443\",\"node-03.do-ams3.eth.prod\":\"enode://ce559a37a9c344d7109bd4907802dd690008381d51f658c43056ec36ac043338bd92f1ac6043e645b64953b06f27202d679756a9c7cf62fdefa01b2e6ac5098e@134.209.136.123:443\",\"node-03.gc-us-central1-a.eth.prod\":\"enode://c07aa0deea3b7056c5d45a85bca42f0d8d3b1404eeb9577610f386e0a4744a0e7b2845ae328efc4aa4b28075af838b59b5b3985bffddeec0090b3b7669abc1f3@35.226.92.155:443\",\"node-04.ac-cn-hongkong-c.eth.prod\":\"enode://385579fc5b14e04d5b04af7eee835d426d3d40ccf11f99dbd95340405f37cf3bbbf830b3eb8f70924be0c2909790120682c9c3e791646e2d5413e7801545d353@47.244.221.249:443\",\"node-04.do-ams3.eth.prod\":\"enode://4e0a8db9b73403c9339a2077e911851750fc955db1fc1e09f81a4a56725946884dd5e4d11258eac961f9078a393c45bcab78dd0e3bc74e37ce773b3471d2e29c@134.209.136.101:443\",\"node-04.gc-us-central1-a.eth.prod\":\"enode://0624b4a90063923c5cc27d12624b6a49a86dfb3623fcb106801217fdbab95f7617b83fa2468b9ae3de593ff6c1cf556ccf9bc705bfae9cb4625999765127b423@35.222.158.246:443\",\"node-05.ac-cn-hongkong-c.eth.prod\":\"enode://b77bffc29e2592f30180311dd81204ab845e5f78953b5ba0587c6631be9c0862963dea5eb64c90617cf0efd75308e22a42e30bc4eb3cd1bbddbd1da38ff6483e@47.75.10.177:443\",\"node-05.do-ams3.eth.prod\":\"enode://a8bddfa24e1e92a82609b390766faa56cf7a5eef85b22a2b51e79b333c8aaeec84f7b4267e432edd1cf45b63a3ad0fc7d6c3a16f046aa6bc07ebe50e80b63b8c@178.128.141.249:443\",\"node-05.gc-us-central1-a.eth.prod\":\"enode://a5fe9c82ad1ffb16ae60cb5d4ffe746b9de4c5fbf20911992b7dd651b1c08ba17dd2c0b27ee6b03162c52d92f219961cc3eb14286aca8a90b75cf425826c3bd8@104.154.230.58:443\",\"node-06.ac-cn-hongkong-c.eth.prod\":\"enode://cf5f7a7e64e3b306d1bc16073fba45be3344cb6695b0b616ccc2da66ea35b9f35b3b231c6cf335fdfaba523519659a440752fc2e061d1e5bc4ef33864aac2f19@47.75.221.196:443\",\"node-06.do-ams3.eth.prod\":\"enode://887cbd92d95afc2c5f1e227356314a53d3d18855880ac0509e0c0870362aee03939d4074e6ad31365915af41d34320b5094bfcc12a67c381788cd7298d06c875@178.128.141.0:443\",\"node-06.gc-us-central1-a.eth.prod\":\"enode://282e009967f9f132a5c2dd366a76319f0d22d60d0c51f7e99795a1e40f213c2705a2c10e4cc6f3890319f59da1a535b8835ed9b9c4b57c3aad342bf312fd7379@35.223.240.17:443\",\"node-07.ac-cn-hongkong-c.eth.prod\":\"enode://13d63a1f85ccdcbd2fb6861b9bd9d03f94bdba973608951f7c36e5df5114c91de2b8194d71288f24bfd17908c48468e89dd8f0fb8ccc2b2dedae84acdf65f62a@47.244.210.80:443\",\"node-07.do-ams3.eth.prod\":\"enode://2b01955d7e11e29dce07343b456e4e96c081760022d1652b1c4b641eaf320e3747871870fa682e9e9cfb85b819ce94ed2fee1ac458904d54fd0b97d33ba2c4a4@134.209.136.112:443\",\"node-07.gc-us-central1-a.eth.prod\":\"enode://b706a60572634760f18a27dd407b2b3582f7e065110dae10e3998498f1ae3f29ba04db198460d83ed6d2bfb254bb06b29aab3c91415d75d3b869cd0037f3853c@35.239.5.162:443\",\"node-08.ac-cn-hongkong-c.eth.prod\":\"enode://32915c8841faaef21a6b75ab6ed7c2b6f0790eb177ad0f4ea6d731bacc19b938624d220d937ebd95e0f6596b7232bbb672905ee12601747a12ee71a15bfdf31c@47.75.59.11:443\",\"node-08.do-ams3.eth.prod\":\"enode://0d9d65fcd5592df33ed4507ce862b9c748b6dbd1ea3a1deb94e3750052760b4850aa527265bbaf357021d64d5cc53c02b410458e732fafc5b53f257944247760@178.128.141.42:443\",\"node-08.gc-us-central1-a.eth.prod\":\"enode://e87f1d8093d304c3a9d6f1165b85d6b374f1c0cc907d39c0879eb67f0a39d779be7a85cbd52920b6f53a94da43099c58837034afa6a7be4b099bfcd79ad13999@35.238.106.101:443\"}},\"eth.staging\":{\"boot\":{\"boot-01.ac-cn-hongkong-c.eth.staging\":\"enode://630b0342ca4e9552f50714b6c8e28d6955bc0fd14e7950f93bc3b2b8cc8c1f3b6d103df66f51a13d773b5db0f130661fb5c7b8fa21c48890c64c79b41a56a490@47.91.229.44:443\",\"boot-01.do-ams3.eth.staging\":\"enode://f79fb3919f72ca560ad0434dcc387abfe41e0666201ebdada8ede0462454a13deb05cda15f287d2c4bd85da81f0eb25d0a486bbbc8df427b971ac51533bd00fe@174.138.107.239:443\",\"boot-01.gc-us-central1-a.eth.staging\":\"enode://10a78c17929a7019ef4aa2249d7302f76ae8a06f40b2dc88b7b31ebff4a623fbb44b4a627acba296c1ced3775d91fbe18463c15097a6a36fdb2c804ff3fc5b35@35.238.97.234:443\"},\"mail\":{\"mail-01.ac-cn-hongkong-c.eth.staging\":\"enode://b74859176c9751d314aeeffc26ec9f866a412752e7ddec91b19018a18e7cca8d637cfe2cedcb972f8eb64d816fbd5b4e89c7e8c7fd7df8a1329fa43db80b0bfe@47.52.90.156:443\",\"mail-01.do-ams3.eth.staging\":\"enode://69f72baa7f1722d111a8c9c68c39a31430e9d567695f6108f31ccb6cd8f0adff4991e7fdca8fa770e75bc8a511a87d24690cbc80e008175f40c157d6f6788d48@206.189.240.16:443\",\"mail-01.gc-us-central1-a.eth.staging\":\"enode://e4fc10c1f65c8aed83ac26bc1bfb21a45cc1a8550a58077c8d2de2a0e0cd18e40fd40f7e6f7d02dc6cd06982b014ce88d6e468725ffe2c138e958788d0002a7f@35.239.193.41:443\"},\"rendezvous\":{\"boot-01.ac-cn-hongkong-c.eth.staging\":\"/ip4/47.91.229.44/tcp/30703/ethv4/16Uiu2HAmRnt2Eyoknh3auxh4fJwkRgqkH1gqrWGes8Pk1k3MV4xu\",\"boot-01.do-ams3.eth.staging\":\"/ip4/174.138.107.239/tcp/30703/ethv4/16Uiu2HAm8UZXUHEPZrpJbcQ3yVFH6UtKrwsG6jH4ai72PsbLfVFb\",\"boot-01.gc-us-central1-a.eth.staging\":\"/ip4/35.238.97.234/tcp/30703/ethv4/16Uiu2HAm6G9sDMkrB4Xa5EH3Zx2dysCxFgBTSRzghic3Z9tRFRNE\"},\"whisper\":{\"node-01.ac-cn-hongkong-c.eth.staging\":\"enode://088cf5a93c576fae52f6f075178467b8ff98bacf72f59e88efb16dfba5b30f80a4db78f8e3cb3d87f2f6521746ef4a8768465ef2896c6af24fd77a425e95b6dd@47.52.226.137:443\",\"node-01.do-ams3.eth.staging\":\"enode://914c0b30f27bab30c1dfd31dad7652a46fda9370542aee1b062498b1345ee0913614b8b9e3e84622e84a7203c5858ae1d9819f63aece13ee668e4f6668063989@167.99.19.148:443\",\"node-01.gc-us-central1-a.eth.staging\":\"enode://d3878441652f010326889f28360e69f2d09d06540f934fada0e17b374ce5319de64279aba3c44a5bf807d9967c6d705b3b4c6b03fa70763240e2ee6af01a539e@35.192.0.86:443\"}},\"eth.test\":{\"boot\":{\"boot-01.ac-cn-hongkong-c.eth.test\":\"enode://daae2e72820e86e942fa2a8aa7d6e9954d4043a753483d8bd338e16be82cf962392d5c0e1ae57c3d793c3d3dddd8fd58339262e4234dc966f953cd73b535f5fa@47.52.188.149:443\",\"boot-01.do-ams3.eth.test\":\"enode://9e0988575eb7717c25dea72fd11c7b37767dc09c1a7686f7c2ec577d308d24b377ceb675de4317474a1a870e47882732967f4fa785b02ba95d669b31d464dec0@206.189.243.164:443\",\"boot-01.gc-us-central1-a.eth.test\":\"enode://c1e5018887c863d64e431b69bf617561087825430e4401733f5ba77c70db14236df381fefb0ebe1ac42294b9e261bbe233dbdb83e32c586c66ae26c8de70cb4c@35.188.168.137:443\"},\"mail\":{\"mail-01.ac-cn-hongkong-c.eth.test\":\"enode://619dbb5dda12e85bf0eb5db40fb3de625609043242737c0e975f7dfd659d85dc6d9a84f9461a728c5ab68c072fed38ca6a53917ca24b8e93cc27bdef3a1e79ac@47.52.188.196:443\",\"mail-01.do-ams3.eth.test\":\"enode://e4865fe6c2a9c1a563a6447990d8e9ce672644ae3e08277ce38ec1f1b690eef6320c07a5d60c3b629f5d4494f93d6b86a745a0bf64ab295bbf6579017adc6ed8@206.189.243.161:443\",\"mail-01.gc-us-central1-a.eth.test\":\"enode://707e57453acd3e488c44b9d0e17975371e2f8fb67525eae5baca9b9c8e06c86cde7c794a6c2e36203bf9f56cae8b0e50f3b33c4c2b694a7baeea1754464ce4e3@35.192.229.172:443\"},\"rendezvous\":{\"boot-01.ac-cn-hongkong-c.eth.test\":\"/ip4/47.52.188.149/tcp/30703/ethv4/16Uiu2HAm9Vatqr4GfVCqnyeaPtCF3q8fz8kDDUgqXVfFG7ZfSA7w\",\"boot-01.do-ams3.eth.test\":\"/ip4/206.189.243.164/tcp/30703/ethv4/16Uiu2HAmBCh5bgYr6V3fDuLqUzvtSAsFTQJCQ3TVHT8ta8bTu2Jm\",\"boot-01.gc-us-central1-a.eth.test\":\"/ip4/35.188.168.137/tcp/30703/ethv4/16Uiu2HAm3MUqtGjmetyZ9L4SN2R8oHDWvACUcec25LjtDD5euiRH\"},\"whisper\":{\"node-01.ac-cn-hongkong-c.eth.test\":\"enode://d63cb1b71501a94ec1c2740407a56939c5c5645215326f766b0d4257426bdbb046c984ee63442a248f3a28f5ed187290e34f7e6a186ae6011c8e5eeee5ca9a70@47.52.255.194:443\",\"node-01.do-ams3.eth.test\":\"enode://3caa837173aa6d13070bf509323fd555c1e23fe40a9eface7d057495b1e2bd4f058b62ecc2829fbb8480b32288fe4e13b5a1d5daa296046c514906529d1e42c8@206.189.243.163:443\",\"node-01.gc-us-central1-a.eth.test\":\"enode://9e55a6a683260191b816b48f631a5ab02d5ed17ebb5c77765ec968b57e5690fdfd9c2f0e4ff7fa50ba0ee6ebdb798afb35fe9e2ec68e4edfdf10267b014a8021@35.194.31.108:443\"}},\"wakuv2.prod\":{\"waku\":{\"node-01.ac-cn-hongkong-c.wakuv2.prod\":\"/ip4/0.0.0.0/tcp/30303/p2p/16Uiu2HAmL5okWopX7NqZWBUKVqW8iUxCEmd5GMHLVPwCgzYzQv3e\",\"node-01.do-ams3.wakuv2.prod\":\"/ip4/0.0.0.0/tcp/30303/p2p/16Uiu2HAmL5okWopX7NqZWBUKVqW8iUxCEmd5GMHLVPwCgzYzQv3e\",\"node-01.gc-us-central1-a.wakuv2.prod\":\"/ip4/0.0.0.0/tcp/30303/p2p/16Uiu2HAmVkKntsECaYfefR1V2yCR79CegLATuTPE6B9TxgxBiiiA\"}},\"wakuv2.test\":{\"waku\":{\"node-01.ac-cn-hongkong-c.wakuv2.test\":\"/ip4/47.242.210.73/tcp/30303/p2p/16Uiu2HAmSyrYVycqBCWcHyNVQS6zYQcdQbwyov1CDijboVRsQS37\",\"node-01.do-ams3.wakuv2.test\":\"/ip4/134.209.139.210/tcp/30303/p2p/16Uiu2HAmPLe7Mzm8TsYUubgCAW1aJoeFScxrLj8ppHFivPo97bUZ\",\"node-01.gc-us-central1-a.wakuv2.test\":\"/ip4/104.154.239.128/tcp/30303/p2p/16Uiu2HAmPLe7Mzm8TsYUubgCAW1aJoeFScxrLj8ppHFivPo97bUZ\"}}},\"meta\":{\"hostname\":\"bots-01.ac-cn-hongkong-c.eth.prod\",\"timestamp\":\"2021-01-23T16:00:24.184409\"}}");


	QJsonObject fleetJson = QJsonDocument::fromJson(fleets.toUtf8().data())
								.object()["fleets"]
								.toObject()["eth.prod"]
								.toObject();

	QJsonObject clusterConfig = nodeConfigJson["ClusterConfig"].toObject();

	auto boot = fleetJson["boot"].toObject();
	QJsonArray bootNodes;
	for(auto it = boot.begin(); it != boot.end(); ++it)
		bootNodes << *it;
	clusterConfig["BootNodes"] = bootNodes;

	auto rendezvous = fleetJson["rendezvous"].toObject();
	QJsonArray rendezvousNodes;
	for(auto it = rendezvous.begin(); it != rendezvous.end(); ++it)
		rendezvousNodes << *it;
	clusterConfig["RendezvousNodes"] = rendezvousNodes;

	auto whisper = fleetJson["whisper"].toObject();
	QJsonArray staticNodes;
	for(auto it = whisper.begin(); it != whisper.end(); ++it)
		staticNodes << *it;
	clusterConfig["StaticNodes"] = staticNodes;

	auto mail = fleetJson["mail"].toObject();
	QJsonArray trustedMailserverNodes;
	for(auto it = mail.begin(); it != mail.end(); ++it)
		trustedMailserverNodes << *it;
	clusterConfig["TrustedMailServers"] = trustedMailserverNodes;

	nodeConfigJson["ClusterConfig"] = clusterConfig;

	return nodeConfigJson;
}

QJsonArray getSubAccountData(GeneratedAccount* account)
{
	return QJsonArray{
		QJsonObject{{"public-key", account->derivedKeys[Constants::PathDefaultWallet].publicKey},
					{"address", account->derivedKeys[Constants::PathDefaultWallet].address},
					{"color", "#4360df"},
					{"wallet", true},
					{"path", Constants::PathDefaultWallet},
					{"name", "Status account"}},
		QJsonObject{
			{"public-key", account->derivedKeys[Constants::PathWhisper].publicKey},
			{"address", account->derivedKeys[Constants::PathWhisper].address},
			{"path", Constants::PathWhisper},
			{"name", Utils::generateAlias(account->derivedKeys[Constants::PathWhisper].publicKey)},
			{"identicon",
			 Utils::generateIdenticon(account->derivedKeys[Constants::PathWhisper].publicKey)},
			{"chat", true}}};
}

bool saveAccountAndLogin(GeneratedAccount* genAccount, QString password)
{
	QString hashedPassword = QString::fromUtf8(
		QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));

	storeDerivedAccount(genAccount->id, hashedPassword);

	QString installationId(QUuid::createUuid().toString(QUuid::WithoutBraces));
	QJsonObject accountData(getAccountData(genAccount));
	QJsonObject settings(getAccountSettings(genAccount, installationId));
	QJsonObject nodeConfig(getNodeConfig(installationId));
	QJsonArray subAccountData(getSubAccountData(genAccount));

	qDebug() << SaveAccountAndLogin(Utils::jsonToStr(accountData).toUtf8().data(),
									hashedPassword.toUtf8().data(),
									Utils::jsonToStr(settings).toUtf8().data(),
									Utils::jsonToStr(nodeConfig).toUtf8().data(),
									Utils::jsonToStr(subAccountData).toUtf8().data());

	return true;
	// TODO: clear hashedPassword, genAccount
}

QString OnboardingModel::validateMnemonic(QString mnemonic)
{
	// TODO: clear memory
	const char* result(ValidateMnemonic(mnemonic.toUtf8().data()));
	const QJsonObject obj = QJsonDocument::fromJson(result).object();
	return obj["error"].toString();
}

QString OnboardingModel::importMnemonic(QString mnemonic)
{
	// TODO: clear memory
	QJsonObject obj1{{"mnemonicPhrase", mnemonic}, {"Bip39Passphrase", ""}};
	const char* importResult = MultiAccountImportMnemonic(
		Utils::jsonToStr(obj1).toUtf8().data()); // TODO: clear from memory
	GeneratedAccount acc = jsonObjectToAccount(QJsonDocument::fromJson(importResult).object());

	QJsonObject obj2{{"accountID", acc.id},
					 {"paths",
					  QJsonArray{Constants::PathWalletRoot,
								 Constants::PathEip1581,
								 Constants::PathWhisper,
								 Constants::PathDefaultWallet}}};
	const char* deriveResult = MultiAccountDeriveAddresses(
		Utils::jsonToStr(obj2).toUtf8().data()); // TODO: clear from memory
	setDerivedKeys(acc, QJsonDocument::fromJson(deriveResult).object());

	beginResetModel();
	mData.clear();
	mData << acc;
	endResetModel();

	return acc.id;
}

void OnboardingModel::setup(QString accountId, QString password)
{
	auto genAccount =
		std::find_if(mData.begin(), mData.end(), [accountId](const GeneratedAccount& m) -> bool {
			return m.keyUid == accountId;
		});
	if(genAccount != mData.end())
	{
		QtConcurrent::run([=] {
			bool result(saveAccountAndLogin(genAccount, password));

			// TODO: clear mnemonic from memory in list

			emit accountSaved(result);
		});
	}

	// TODO: error handling
	// TODO: clear password
}