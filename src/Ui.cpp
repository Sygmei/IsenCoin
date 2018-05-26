#include <Chain.hpp>
#include <Config.hpp>
#include <Ui.hpp>

#include <imgui/imgui-SFML.h>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

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

    std::vector<Wallet>& WalletList::get_wallets()
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
    }

    CreateWalletWindow::CreateWalletWindow() : UiWindow("Wallet Creation")
    {
        wallet_prefix_input.resize(20);
        wallet_name_input.resize(20);
    }

    void CreateWalletWindow::update(WalletList& wallets)
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
                ic::Log->error("Creating Wallet with following prefix : '{}'", wallet_prefix);
                creating_wallet = true;
                std::thread wallet_creation_thread([&]()
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
    }

    void TxExplorerWindow::update()
    {
        if (m_opened)
        {
            ImGui::Begin(m_title.c_str(), &m_opened,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize);
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
        CreateWalletWindow win_wallet_creation;
        AddNodeWindow win_node_add;

        int selected_wallet;
        int selected_other_wallet;
        int selected_node;
        bool save_nodes = true;
        int tracker_port_input = config::DEFAULT_PORT;
        int threads_input = 1;
        sf::Texture mining_status_tex;

        Wallet myWallet;
        Wallet myWallet2;
        myWallet.generate();
        myWallet2.generate();

        Transaction tx(myWallet, myWallet2, 22);
        tx.validate();
        Transaction tx2(myWallet, myWallet2, 11);
        Transaction tx3(myWallet, myWallet2, 53);
        tx3.validate();
        Transaction tx4(myWallet, myWallet2, 958);
        Chain::Blockchain().get_current_block().add_transaction(tx);
        Chain::Blockchain().get_current_block().add_transaction(tx2);
        Chain::Blockchain().get_current_block().add_transaction(tx3);
        Chain::Blockchain().get_current_block().add_transaction(tx4);
        
        TxDisplayWindow tx_display_window(tx);
        mining_status_tex.loadFromFile("ui/cog.png");

        std::vector<std::unique_ptr<UiWindow>> free_windows;
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

            win_wallet_creation.update(wallets);
            win_node_add.update(tracker);

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
            ImGui::ListBox("##Wallets", &selected_wallet, vector_getter, static_cast<void*>(&wallets.get_names()), wallets.get_amount());
            if (ImGui::Button("Create a wallet")) win_wallet_creation.toggle();
            ImGui::Text("Other's wallets :");
            ImGui::ListBox("##OtherWallets", &selected_other_wallet, vector_getter, static_cast<void*>(&wallets.get_names()), wallets.get_amount());
            ImGui::Button("Add an address");
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
            if (selected_wallet >= 0)
            {
                const std::string current_wallet_name = wallets.get_wallet_name_at_index(selected_wallet);
                Wallet& current_wallet = wallets.get_wallet_at_index(selected_wallet);

                ImGui::Text(fmt::format("Wallet Name : {}", current_wallet_name).c_str());
                ImGui::Text("Wallet Private Key :");
                ImGui::TextWrapped(current_wallet.get_b58_private_key().c_str());
                ImGui::Text("Wallet Public Key :");
                ImGui::TextWrapped(current_wallet.get_b58_public_key().c_str());
                ImGui::Text(fmt::format("Wallet balance : {} ic", 22.424).c_str());
                ImGui::Button("Create A Transaction");
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
            if (ImGui::Button("Add Node")) win_node_add.toggle();
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
            ImGui::SliderInt("Threads", &threads_input, 1, 8);
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
                    Chain::Blockchain().mine_and_next(true);
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
            ImGui::Button("Block Explorer");
            ImGui::End();

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
