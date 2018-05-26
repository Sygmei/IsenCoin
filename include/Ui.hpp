#pragma once

#include <string>
#include <vector>

#include <base58/base58.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui-SFML.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include <Block.hpp>
#include <Functions.hpp>
#include <ImguiTxViewer.hpp>
#include <Logger.hpp>
#include <Tracker.hpp>
#include <Wallet.hpp>

namespace ic::ui
{
    static std::string loading_dots;
    void update_loading_dots(std::string& loading_dots);

    class UiWindow
    {
    protected:
        bool m_opened = false;
        std::string m_title;
    public:
        explicit UiWindow(const std::string& title);
        std::string get_title() const;
        void open();
        void close();
        void toggle();
        bool is_opened() const;
        virtual void update() {};
    };

    class WalletList
    {
    private:
        std::vector<std::pair<std::string, Wallet>> m_wallets;
    public:
        std::string get_wallet_name_at_index(unsigned int index);
        Wallet& get_wallet_at_index(unsigned int index);
        std::vector<std::pair<std::string, Wallet>>& get();
        std::vector<Wallet>& get_wallets();
        std::vector<std::string> get_names();
        unsigned int get_amount() const;
        void add_wallet(const std::string& name, Wallet& wallet);
    };

    class CreateWalletWindow : public UiWindow
    {
    private:
        int creating_wallet = 0;
        std::string wallet_prefix_input;
        std::string wallet_name_input;
        Wallet wallet_creator{};
    public:
        CreateWalletWindow();
        void update(WalletList& wallets);
    };

    class TxDisplayWindow : public UiWindow
    {
    private:
        Transaction* m_transaction;
        ImGui::ext::TxViewer tx_viewer;
        std::string m_title;
    public:
        TxDisplayWindow(Transaction& tx);
        void update() override;
    };

    class TxExplorerWindow : public UiWindow
    {
    private:
        Block* m_block;
        std::vector<std::unique_ptr<UiWindow>>* m_free_windows;
        int m_selected_tx = 0;
    public:
        TxExplorerWindow(std::vector<std::unique_ptr<UiWindow>>* windows, Block& block);
        void update() override;
    };

    class BlockExplorerWindow : public UiWindow
    {
    public:
        BlockExplorerWindow();
        void update();
    };

    class AddNodeWindow : public UiWindow
    {
    private:
        std::string node_address_input;
        int node_port_input;
    public:
        AddNodeWindow();
        void update(Tracker& tracker);
    };

    void main(Tracker& tracker);
}
