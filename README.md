# IsenCoin

IsenCoin is a cryptocurrency prototype built using modern C++ !

⚠️ This project is available for learning purpose only ! IsenCoin is not production ready nor it will ever be.

## Features

- Proof Of Work block validation
- P2P with binary serialization of packets
- Wallet with graphical interface
- Mining software integrated

## Dependencies

| Library | Purpose | Link |
|---------|-------------|------|
| base58  | Generates wallet addresses with non-ambiguous characters | (Edited version of [Bitcoin's base58](https://github.com/bitcoin/bitcoin/blob/119b0f85e2c8b9729228aad5d946144d57ad0f5b/src/base58.cpp)) |
| ed25519 | Signs and verifies transactions | https://github.com/orlp/ed25519 |
| fmt | String formatting | https://github.com/fmtlib/fmt |
| imgui | Wallet GUI | https://github.com/ocornut/imgui |
| msgpack11 | Binary serialization of network packets | https://github.com/ar90n/msgpack11 |
| SFML | Window / Mouse support | https://github.com/SFML/SFML |
| spglog | Logging | https://github.com/gabime/spdlog |
| tacopie | TCP communication | https://github.com/Cylix/tacopie |
| vili | Human readable data language | https://github.com/Sygmei/Vili |

## Screenshots

### Main menu

![Main menu screenshot](https://raw.githubusercontent.com/Sygmei/IsenCoin/master/screenshots/main_menu.png)

### Transaction creator

![Transaction creator screenshot](https://raw.githubusercontent.com/Sygmei/IsenCoin/master/screenshots/create_transaction.png)

### Blockchain explorer

![Blockchain explorer screenshot](https://github.com/Sygmei/IsenCoin/blob/master/screenshots/blockchain_explorer.png)

### Block explorer

![Block explorer screenshot](https://github.com/Sygmei/IsenCoin/blob/master/screenshots/block_explorer.png)

### Transaction viewer

![Transaction viewer screenshot](https://github.com/Sygmei/IsenCoin/blob/master/screenshots/transaction_viewer.png)
