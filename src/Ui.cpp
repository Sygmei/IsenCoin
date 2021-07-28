#include <Chain.hpp>
#include <Config.hpp>
#include <Ui.hpp>

#include <imgui/imgui-SFML.h>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include "vili/ViliParser.hpp"

namespace ic::ui
{
    static unsigned int unique_id_gen = 0;
    std::string unique_win(const std::string& title)
    {
        std::string proutch = title + "##" + std::to_string(unique_id_gen++);
        Log->error("Generated unique win id : {}", proutch);
        return proutch;
    }

    bool check_for_free_window(std::vector<std::unique_ptr<UiWindow>>* free_windows, const std::string& title)
    {
        for (const auto& win : *free_windows)
        {
            if (win->get_title() == title)
                return true;
        }
        return false;
    }

    void update_loading_dots(std::string& loading_dots)
    {
        using namespace std::chrono_literals;
        unsigned int step = 0;
        for (;;)
        {
            loading_dots = "";
            for (unsigned int i = 0; i < step + 1; i++)
            {
                loading_dots += ".";
            }
            step++;
            std::this_thread::sleep_for(1s);
            if (step == 3) step = 0;
        }
    }

    bool vector_getter(void* vec, int idx, const char** out_text)
    {
        auto& vector = *static_cast<std::vector<std::string>*>(vec);
        if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
        *out_text = vector.at(idx).c_str();
        return true;
    };

    UiWindow::UiWindow(const std::string& title)
    {
        m_title = title;
    }

    std::string UiWindow::get_title() const
    {
        return m_title;
    }

    void UiWindow::open()
    {
        m_opened = true;
    }

    void UiWindow::close()
    {
        m_opened = false;
    }

    void UiWindow::toggle()
    {
        m_opened = !m_opened;
    }

    bool UiWindow::is_opened() const
    {
        return m_opened;
    }

    void WalletList::save()
    {
        vili::ViliParser dump_wallets;
        for (const auto& wallet : m_wallets)
        {
            dump_wallets->createComplexNode(wallet.first);
            dump_wallets->at(wallet.first).createDataNode("public", wallet.second.get_b58_public_key());
            dump_wallets->at(wallet.first).createDataNode("private", wallet.second.get_b58_private_key());
        }
        dump_wallets.writeFile("wallets.vili");
    }

    WalletList::WalletList()
    {
        vili::ViliParser load_wallets;
        load_wallets.parseFile("wallets.vili");
        for (const auto* wallet : load_wallets->getAll<vili::ComplexNode>())
        {
            std::vector<unsigned char> pub_key_dcode;
            std::vector<unsigned char> priv_key_dcode;
            std::string pub_key_vili = wallet->at<vili::DataNode>("public").get<std::string>();
            std::string priv_key_vili = wallet->at<vili::DataNode>("private").get<std::string>();
            base58::decode(pub_key_vili.c_str(), pub_key_dcode);
            base58::decode(priv_key_vili.c_str(), priv_key_dcode);
            private_key_t private_key;
            public_key_t public_key;
            std::copy(pub_key_dcode.begin(), pub_key_dcode.end(), public_key.begin());
            std::copy(priv_key_dcode.begin(), priv_key_dcode.end(), private_key.begin());
            Wallet new_wallet(private_key, public_key);
            add_wallet(wallet->getId(), new_wallet);
        } 
    }

    std::string WalletList::get_wallet_name_at_index(unsigned index)
    {
        return m_wallets[index].first;
    }

    Wallet& WalletList::get_wallet_at_index(unsigned index)
    {
        return m_wallets[index].second;
    }

    std::vector<std::pair<std::string, Wallet>>& WalletList::get()
    {
        return m_wallets;
    }

    std::vector<Wallet> WalletList::get_wallets()
    {
        std::vector<Wallet> result;
        for (const auto& w : m_wallets)
        {
            result.push_back(w.second);
        }
        return result;
    }

    std::vector<std::string> WalletList::get_names()
    {
        std::vector<std::string> result;
        for (const auto& w : m_wallets)
            result.push_back(w.first);
        return result;
    }

    unsigned WalletList::get_amount() const
    {
        return m_wallets.size();
    }

    void WalletList::add_wallet(const std::string& name, Wallet& wallet)
    {
        m_wallets.emplace_back(name, wallet);
        save();
    }

    void AddressBook::save()
    {
        vili::ViliParser dump_addresses;
        for (const auto& address : m_addresses)
            dump_addresses->createDataNode(address.first, address.second);
        dump_addresses.writeFile("addresses.vili");
    }

    AddressBook::AddressBook()
    {
        vili::ViliParser load_addresses;
        load_addresses.parseFile("addresses.vili");
        for (const auto* address : load_addresses->getAll<vili::DataNode>())
            add_address(address->getId(), address->get<std::string>());
    }

    std::string AddressBook::get_address_name_at_index(unsigned int index)
    {
        return m_addresses[index].first;
    }

    public_key_t AddressBook::get_public_key_at_index(unsigned int index)
    {
        public_key_t pkey;
        std::vector<unsigned char> pkey_vctor;
        base58::decode(m_addresses[index].second.c_str(), pkey_vctor);
        std::copy(pkey_vctor.begin(), pkey_vctor.end(), pkey.begin());
        return pkey;
    }

    std::vector<std::pair<std::string, std::string>>& AddressBook::get()
    {
        return m_addresses;
    }

    std::vector<public_key_t> AddressBook::get_public_keys()
    {
        std::vector<public_key_t> pkeys;
        for (unsigned int i = 0; i < get_amount(); i++)
            pkeys.push_back(get_public_key_at_index(i));
        return pkeys;
    }

    std::vector<std::string> AddressBook::get_names()
    {
        std::vector<std::string> names;
        for (unsigned int i = 0; i < get_amount(); i++)
            names.push_back(get_address_name_at_index(i));
        return names;
    }

    unsigned AddressBook::get_amount() const
    {
        return m_addresses.size();
    }

    void AddressBook::add_address(const std::string& name, const std::string& public_key)
    {
        m_addresses.emplace_back(name, public_key);
        save();
    }

    CreateWalletWindow::CreateWalletWindow() : UiWindow("Wallet Creation")
    {
        wallet_prefix_input.resize(20);
        wallet_name_input.resize(20);
    }

    void CreateWalletWindow::update(WalletList& wallets, AddressBook& address_book)
    {
        if (m_opened)
        {
            ImGui::Begin(m_title.c_str(), &m_opened,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::InputText("prefix", wallet_prefix_input.data(), wallet_prefix_input.size());
            if (ImGui::Button("Create Wallet"))
            {
                std::string wallet_prefix = utils::rtrim(wallet_prefix_input);
                for (const char& c : wallet_prefix)
                {
                    if (base58::alphabet.find(c) == std::string::npos)
                    {
                        wallet_prefix = "";
                        Log->warn("Invalid Prefix character (not in Base58)");
                    }
                }
                ic::Log->info("Creating Wallet with following prefix : '{}'", wallet_prefix);
                creating_wallet = true;
                std::thread wallet_creation_thread([wallet_prefix, this]()
                {
                    wallet_creator.generate(wallet_prefix);
                    creating_wallet = 2;
                });
                wallet_creation_thread.detach();
            }
            if (creating_wallet == 2)
            {
                ImGui::Text(fmt::format("Wallet Created ! {}", wallet_creator.get_b58_public_key()).c_str());
                ImGui::Text("Name of the wallet : ");
                ImGui::SameLine();
                ImGui::InputText("##wallet_name", wallet_name_input.data(), wallet_name_input.size());
                ImGui::SameLine();
                if (ImGui::Button("Keep this wallet !"))
                {
                    const std::string wallet_name = utils::rtrim(wallet_name_input);
                    wallet_name_input = std::string(20, 0);
                    wallet_prefix_input = std::string(20, 0);
                    Log->error("WC : {}", wallet_creator.get_b58_public_key());
                    wallets.add_wallet(wallet_name, wallet_creator);
                    address_book.add_address(wallet_name, wallet_creator.get_b58_public_key());
                    creating_wallet = 0;
                    close();
                }
            }
            else if (creating_wallet == 1)
                ImGui::Text(("Creating Wallet" + loading_dots).c_str());
            else if (creating_wallet == 0)
                ImGui::Text("Click the 'Create Wallet' button to create a Wallet");
            ImGui::End();
        }
    }

    AddAddressWindow::AddAddressWindow() : UiWindow("Add Address")
    {
        pub_key_input.resize(45);
        key_name_input.resize(20);
    }

    void AddAddressWindow::update(AddressBook& address_book)
    {
        if (m_opened)
        {
            ImGui::Begin(m_title.c_str(), &m_opened,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Address Label :");
            ImGui::InputText("##key_name", key_name_input.data(), key_name_input.size());
            ImGui::Text("Public Key :");
            ImGui::InputText("##pub_key", pub_key_input.data(), pub_key_input.size());
            if (ImGui::Button("Add Address"))
            {
                const std::string pub_key = utils::rtrim(pub_key_input);
                const std::string key_name = utils::rtrim(key_name_input);
                address_book.add_address(key_name, pub_key);
            }
            ImGui::End();
        }
    }

    TxDisplayWindow::TxDisplayWindow(Transaction& tx) : UiWindow("")
    {
        m_transaction = &tx;
        tx_viewer.ReadOnly = true;
        tx_viewer.tx_validity = tx.validate();
        m_opened = true;
        m_title = unique_win("Tx Raw Memory Viewer");
        Log->error("BUILDING TXDISPLAYWINDOW {}", m_title);
    }

    void TxDisplayWindow::update()
    {
        if (m_opened)
        {
            tx_viewer.DrawWindow(m_title,
                m_transaction,
                reinterpret_cast<unsigned char*>(m_transaction->get_signable_transaction_message().data()), 
                m_transaction->get_signable_transaction_message().size(), &m_opened);
        }
    }

    TxExplorerWindow::TxExplorerWindow(std::vector<std::unique_ptr<UiWindow>>* windows, Block& block) : 
    UiWindow("Block Explorer #" + std::to_string(block.get_depth()))
    {
        m_block = &block;
        m_free_windows = windows;
        m_opened = true;
        signature_t merkle_root = m_block->calculate_merkle_root();
        merkle_root_b58 = base58::encode(merkle_root.data(), merkle_root.data() + merkle_root.size());
    }

    void TxExplorerWindow::update()
    {
        if (m_opened)
        {
            ImGui::Begin(m_title.c_str(), &m_opened,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text(fmt::format("Block Depth : {}", m_block->get_depth()).c_str());
            ImGui::Text(fmt::format("Validity : {}", m_block->is_valid() ? "Valid" : "Invalid").c_str());
            ImGui::TextWrapped(fmt::format("Block Merkle Root : {}", merkle_root_b58).c_str());
            if (m_block->is_valid())
            {
                ImGui::Text(fmt::format("Block nonce : {}", m_block->get_nonce()).c_str());
                ImGui::Text(fmt::format("Block timestamp : {}", m_block->get_timestamp()).c_str());
                ImGui::TextWrapped(fmt::format("Block hash : {}", m_block->get_hex_hash()).c_str());
                std::string reward_to;
                for (const auto& tx : m_block->get_transactions())
                {
                    if (tx->is_reward())
                    {
                        reward_to = base58::encode(tx->get_receiver().data(), tx->get_receiver().data() + tx->get_receiver().size());
                    }
                }
                if (!reward_to.empty())
                    ImGui::Text(fmt::format("Reward to : {}", reward_to).c_str());
            }
            ImGui::Text(fmt::format("Transaction amount : {}", m_block->get_tx_amount()).c_str());
            ImGui::Text("Transaction List : ");
            std::vector<std::string> tx_list;
            for (const auto& tx : m_block->get_transactions())
                tx_list.push_back(tx->get_printable_signature().substr(0, 20) + "...");
            ImGui::ListBox("##tx_list", &m_selected_tx, vector_getter, static_cast<void*>(&tx_list), m_block->get_tx_amount(), 5);
            if (!tx_list.empty())
            {
                ImGui::TextWrapped(fmt::format("Full signature : {}", m_block->get_transactions()[m_selected_tx]->get_printable_signature()).c_str());
            }
            if (ImGui::Button("View Transaction"))
            {
                m_free_windows->push_back(std::make_unique<TxDisplayWindow>(*m_block->get_transactions()[m_selected_tx]));
            }
            ImGui::End();
        }
    }

    BlockchainExplorerWindow::BlockchainExplorerWindow(std::vector<std::unique_ptr<UiWindow>>* windows) :
    UiWindow("Blockchain Explorer")
    {
        m_free_windows = windows;
    }

    void BlockchainExplorerWindow::update()
    {
        if (m_opened)
        {
            ImGui::Begin(m_title.c_str(), &m_opened,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Block List : ");
            std::vector<std::string> blk_list;
            for (const auto& blk : Chain::Blockchain().get_blocks())
            {
                if (blk->is_valid())
                    blk_list.push_back(blk->get_hex_hash().substr(0, 20) + "...");
            }

            ImGui::ListBox("##blk_list", &m_selected_block, vector_getter, static_cast<void*>(&blk_list), blk_list.size(), 5);
            if (!blk_list.empty())
            {
                ImGui::TextWrapped(fmt::format("Full Hash : {}", Chain::Blockchain().get_blocks()[m_selected_block]->get_hex_hash()).c_str());
            }
            if (ImGui::Button("View Block"))
            {
                m_free_windows->push_back(std::make_unique<TxExplorerWindow>(m_free_windows, *Chain::Blockchain().get_blocks()[m_selected_block]));
            }
            ImGui::End();
        }
    }

    void push_red_button_style()
    {
        const ImVec4 red(0.7f, 0, 0, 1.f);
        const ImVec4 dark_red(0.5f, 0, 0, 1.f);
        const ImVec4 bright_red(1.f, 0, 0, 1.f);
        ImGui::PushStyleColor(ImGuiCol_Button, red);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, dark_red);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, bright_red);
    }

    AddNodeWindow::AddNodeWindow() : UiWindow("Add Node")
    {
        node_address_input.resize(16);
        node_port_input = config::DEFAULT_PORT;
    }

    void AddNodeWindow::update(Tracker& tracker)
    {
        if (m_opened)
        {
            ImGui::Begin(m_title.c_str(), &m_opened,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.35f);
            ImGui::InputText("Node address", node_address_input.data(), node_address_input.size());
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.22f);
            ImGui::InputInt("Node Port", &node_port_input);
            ImGui::PopItemWidth();

            if (ImGui::Button("Add Node"))
            {
                const std::string node_address = utils::rtrim(node_address_input);
                const Node new_node(node_address, node_port_input);
                tracker.add_node(new_node);
            }

            ImGui::End();
        }
    }

    TxCreateWindow::TxCreateWindow() : UiWindow("Create Transaction")
    {
    }

    void TxCreateWindow::update(const std::string& sender_name, Wallet& current_wallet, AddressBook& address_book, Tracker& tracker)
    {
        if (m_opened)
        {
            ImGui::Begin(m_title.c_str(), &m_opened,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize);
            if (success == 0)
            {
                ImGui::Text(fmt::format("Sender : {}", sender_name).c_str());
                ImGui::Text("Receiver :");
                std::vector<std::string> address_book_names = address_book.get_names();
                ImGui::ListBox("##wallet_receiver", &m_selected_receiver, vector_getter, static_cast<void*>(&address_book_names), address_book.get_amount());
                ImGui::InputFloat("Amount", &m_amount);
                if (ImGui::Button("Send"))
                {
                    const Transaction new_tx(current_wallet, address_book.get_public_key_at_index(m_selected_receiver), m_amount);
                    if (current_wallet.get_funds() >= m_amount)
                    {
                        Chain::Blockchain().get_current_block().add_transaction(new_tx);
                        Log->error("Mining with reward from ui : {}", current_wallet.get_b58_public_key());
                        Chain::Blockchain().mine_and_next(tracker, false, current_wallet.get_public_key());
                        tracker.propagate(new_tx.to_msgpack());
                        success = 1;
                    }
                    else
                    {
                        success = 2;
                    }
                }
            }
            else if (success == 1)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                ImGui::Text("Transaction sent !");
                ImGui::PopStyleColor();
                if (ImGui::Button("OK"))
                {
                    success = 0;
                }
            }
            else if (success == 2)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::Text("Not enough funds !");
                ImGui::PopStyleColor();
                if (ImGui::Button("OK"))
                {
                    success = 0;
                }
            }
            ImGui::End();
        }
    }

    void main(Tracker& tracker)
    {
        using namespace ic;

        sf::Vector2u w_size(640, 480);
        sf::RenderWindow window(sf::VideoMode(w_size.x, w_size.y), "IsenCoin Panel");
        window.setFramerateLimit(60);
        ImGui::SFML::Init(window);

        static auto nodes_to_string = [](const std::vector<Node>& nodes)
        {
            std::vector<std::string> n_str;
            for (const auto& node : nodes)
                n_str.emplace_back(node.get_address() + ":" + std::to_string(node.get_port()));
            return n_str;
        };

        static auto close_orphaned_windows = [](std::vector<std::unique_ptr<UiWindow>>& windows)
        {
            windows.erase(std::remove_if(windows.begin(), windows.end(), [](const std::unique_ptr<UiWindow>& win)
            {
                if (!win->is_opened())
                    Log->warn("Removing orphaned window");
                return !win->is_opened();
            }), windows.end());
        };

        sf::Clock deltaClock;

        std::thread loading_dots_thread([] { return update_loading_dots(loading_dots); });

        WalletList wallets;
        AddressBook address_book;

        int selected_wallet = 0;
        int selected_other_wallet;
        int selected_node;
        bool save_nodes = true;
        int tracker_port_input = tracker.get_port();
        int threads_input = 1;
        sf::Texture mining_status_tex;

        mining_status_tex.loadFromFile("ui/cog.png");

        std::vector<std::unique_ptr<UiWindow>> free_windows;
        CreateWalletWindow wallet_creation_win;
        AddNodeWindow node_add_win;
        BlockchainExplorerWindow blockchain_exp_win(&free_windows);
        TxCreateWindow tx_create_win;
        AddAddressWindow add_address_win;
        //free_windows.push_back(std::make_unique<TxDisplayWindow>(tx));

        ImGui::GetIO().IniFilename = nullptr;
        while (window.isOpen()) {
            w_size = window.getSize();
            sf::Event event;
            while (window.pollEvent(event)) {
                ImGui::SFML::ProcessEvent(event);

                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            ImGui::SFML::Update(window, deltaClock.restart());

            //Wallets
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(w_size.x / 4, w_size.y));
            ImGui::Begin("Wallets", nullptr,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoCollapse);
            ImGui::Text("Your wallets :");
            std::vector<std::string> wallet_names = wallets.get_names();
            ImGui::ListBox("##Wallets", &selected_wallet, vector_getter, static_cast<void*>(&wallet_names), wallets.get_amount());
            if (ImGui::Button("Create a wallet")) wallet_creation_win.toggle();
            ImGui::Text("Other's wallets :");
            std::vector<std::string> address_book_names = address_book.get_names();
            ImGui::ListBox("##OtherWallets", &selected_other_wallet, vector_getter, static_cast<void*>(&address_book_names), address_book.get_amount());
            if (ImGui::Button("Add an address")) add_address_win.toggle();
            ImGui::End();

            //Wallet Menu
            ImGui::SetNextWindowPos(ImVec2(w_size.x / 4, 0));
            ImGui::SetNextWindowSize(ImVec2(w_size.x - w_size.x / 4, w_size.y / 2));
            ImGui::Begin("Wallet Menu", nullptr,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoCollapse);
            if (wallets.get_amount() > 0 && selected_wallet >= 0)
            {
                const std::string current_wallet_name = wallets.get_wallet_name_at_index(selected_wallet);
                Wallet& current_wallet = wallets.get_wallet_at_index(selected_wallet);

                ImGui::Text(fmt::format("Wallet Name : {}", current_wallet_name).c_str());
                ImGui::Text("Wallet Private Key :");
                ImGui::TextWrapped(current_wallet.get_b58_private_key().c_str());
                ImGui::Text("Wallet Public Key :");
                ImGui::TextWrapped(current_wallet.get_b58_public_key().c_str());
                std::string pub_key_copy = current_wallet.get_b58_public_key();
                ImGui::InputText("Public Key Field", 
                    pub_key_copy.data(), 
                    current_wallet.get_b58_public_key().size());
                ImGui::Text(fmt::format("Wallet balance : {} ic", current_wallet.get_funds()).c_str());
                if (ImGui::Button("Create A Transaction"))
                {
                    tx_create_win.toggle();
                }
                tx_create_win.update(current_wallet_name, current_wallet, address_book, tracker);
                push_red_button_style();
                ImGui::Button("Delete Wallet /!\\");
                ImGui::PopStyleColor(3);
            }
            ImGui::End();

            //Network
            ImGui::SetNextWindowPos(ImVec2(w_size.x / 4, w_size.y / 2));
            ImGui::SetNextWindowSize(ImVec2(w_size.x * 3 / 8, w_size.y / 2));
            ImGui::Begin("Network Menu", nullptr,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoCollapse);
            ImGui::Text(fmt::format("Connected Nodes : {}", tracker.get_nodes().size()).c_str());
            std::vector<std::string> nodes_address = nodes_to_string(tracker.get_nodes());
            ImGui::ListBox("##node_list", &selected_node, vector_getter, static_cast<void*>(&nodes_address), tracker.get_nodes().size(), 5);
            ImGui::Checkbox("Save Network Nodes", &save_nodes);
            if (ImGui::Button("Add Node")) node_add_win.toggle();
            ImGui::SameLine();
            push_red_button_style();
            if (ImGui::Button("Remove Node") && !tracker.get_nodes().empty())
            {
                Log->warn("Removing Node at index {}", selected_node);
                tracker.remove_node_at_index(selected_node);
            }
            ImGui::PopStyleColor(3);
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
            ImGui::InputInt("Tracker port", &tracker_port_input);
            ImGui::PopItemWidth();
            if (ImGui::Button("Apply Port"))
            {
                if (tracker_port_input < 0 || tracker_port_input > std::numeric_limits<uint16_t>::max())
                    tracker_port_input = config::DEFAULT_PORT;
                uint16_t tracker_port_uint = tracker_port_input;
                Log->warn("Restarting Tracker with port {}", tracker_port_uint);
                tracker.set_port(tracker_port_uint);
            }
            ImGui::End();

            //Settings
            ImGui::SetNextWindowPos(ImVec2(w_size.x * 5 / 8, w_size.y / 2));
            ImGui::SetNextWindowSize(ImVec2(w_size.x * 3 / 8, w_size.y / 2));
            ImGui::Begin("Mining / Block Menu", nullptr,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoCollapse); 
            ImGui::Text(fmt::format("Pending Transactions : {}", Chain::Blockchain().get_current_block().get_tx_amount()).c_str());
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);
            ImGui::SliderInt("Threads", &Block::mining_threads, 1, 8);
            ImGui::PopItemWidth();
            ImGui::Checkbox("Start Mining Automatically", &Chain::Blockchain().auto_mining);
            if (Chain::Blockchain().auto_mining)
            {
                ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
                ImGui::SliderInt("Min tx amount", &Chain::Blockchain().minimum_tx_amount, 1, 100);
                ImGui::PopItemWidth();
            }
            else
            {
                if (ImGui::Button("Start mining now"))
                {
                    Log->warn("Start mining now...");
                    if (wallets.get_amount() > 0 && selected_wallet >= 0)
                    {
                        Wallet& current_wallet = wallets.get_wallet_at_index(selected_wallet);
                        Chain::Blockchain().mine_and_next(tracker, true, current_wallet.get_public_key());
                    }
                    else
                        Chain::Blockchain().mine_and_next(tracker, true);
                }
            }
            ImGui::Text("Mining status :");
            ImGui::SameLine();
            if (Chain::Blockchain().get_current_block().is_mining())
                ImGui::Image(mining_status_tex, sf::Vector2f(16, 16), sf::Color::Green);
            else
                ImGui::Image(mining_status_tex, sf::Vector2f(16, 16), sf::Color::Red);
            if (ImGui::Button("Transactions Explorer"))
            {
                if (!check_for_free_window(&free_windows, "Block Explorer #" + std::to_string(Chain::Blockchain().get_current_block().get_depth())))
                    free_windows.push_back(std::make_unique<TxExplorerWindow>(&free_windows, Chain::Blockchain().get_current_block()));
            }
            if (ImGui::Button("Block Explorer"))
                blockchain_exp_win.toggle();
            if (ImGui::Button("Force Sync"))
                Chain::Blockchain().fetch_genesis(tracker);
            ImGui::End();

            wallet_creation_win.update(wallets, address_book);
            node_add_win.update(tracker);
            blockchain_exp_win.update();
            add_address_win.update(address_book);

            const unsigned int f_win_size = free_windows.size();
            for (unsigned int i = 0; i < f_win_size; i++)
                free_windows[i]->update();
            close_orphaned_windows(free_windows);

            window.clear();
            ImGui::SFML::Render(window);
            window.display();
        }

        ImGui::SFML::Shutdown();
    }
}
