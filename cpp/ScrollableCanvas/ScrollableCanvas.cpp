
#include <functional>
#include <vector>
#include <TGUI/TGUI.hpp>

class CanvasUpdater
{
private:
    using CanvasInfo = std::pair< tgui::Canvas::Ptr, std::function< void() > >;
    using CanvasesInfo = std::vector< CanvasInfo >;
    
    std::function< bool() > IsOpenFunc;
    CanvasesInfo Items;
public:

    CanvasUpdater(std::function< bool() > Handler)
        :IsOpenFunc(Handler)
    {}
    
    void add(const CanvasInfo& Item)
    {
        if(Item.first == nullptr)
        {    
            throw std::runtime_error("Bad shared pointer");
        }
        
        Items.emplace_back(Item);
    }

    void RemoveOrphans()
    {
        for(auto Item = Items.begin(); Item != Items.end();)
        {
            if(Item->first.use_count() == 1)
            {
                std::puts("Destroy");
                Item = Items.erase(Item);
            }
            else
                Item++;
        }
    }

    void Update()
    {
        if(IsOpenFunc() == false)
            return;

        for(CanvasInfo& Item : Items)
        { 
            if(Item.second)
                Item.second();
        }
    }
};

namespace Handlers
{
    void Handlers::HandleVScrollbar(tgui::Canvas::Ptr Canvas, tgui::Scrollbar::Ptr VerticalScroll)
    {
        sf::View CanvasView;
        uint ViewportSize = VerticalScroll->getViewportSize() / 2;
        uint Value = VerticalScroll->getValue();
        CanvasView.setCenter(Canvas->getView().getCenter().x, Value + ViewportSize);
        CanvasView.setSize(Canvas->getSize());
        Canvas->setView(CanvasView);
    }

    void Handlers::HandleHScrollbar(tgui::Canvas::Ptr Canvas, tgui::Scrollbar::Ptr HorizontalScroll)
    {
        sf::View CanvasView;
        uint Value = HorizontalScroll->getValue();
        uint ViewportSize = HorizontalScroll->getViewportSize() / 2;
        CanvasView.setCenter(Value + ViewportSize, Canvas->getView().getCenter().y);
        CanvasView.setSize(Canvas->getSize());
        Canvas->setView(CanvasView);
    }

    void Handlers::Handle2DList(tgui::ListView::Ptr ListView)
    {
        std::size_t ColumnsCount = ListView->getColumnCount();
        float ColumnWidth = ListView->getSize().x / ColumnsCount;

        for (size_t Index = 0; Index < ColumnsCount - 1; Index++)
            ListView->setColumnWidth(Index, ColumnWidth);
    }

    void Handlers::ScaleScrollableCanvas(tgui::Group::Ptr BoxLayout, tgui::Canvas::Ptr Canvas, 
        tgui::Scrollbar::Ptr VerticalScroll, tgui::Scrollbar::Ptr HorizontalScroll)
    {
        sf::Vector2f CanvasSize = BoxLayout->getSize() - sf::Vector2f(20.f, 20.f);
        Canvas->setSize(CanvasSize);
        Canvas->setView(sf::View({
            static_cast< float >(HorizontalScroll->getValue()), 
            static_cast< float >(VerticalScroll->getValue()), 
            CanvasSize.x, CanvasSize.y
        }));

        VerticalScroll->setPosition({CanvasSize.x , 0.f});
        VerticalScroll->setSize({20.f, CanvasSize.y});
        VerticalScroll->setViewportSize(static_cast< uint >(CanvasSize.y));

        HorizontalScroll->setPosition({0.f, CanvasSize.y});
        HorizontalScroll->setSize({CanvasSize.x, 20.f});
        HorizontalScroll->setViewportSize(static_cast< uint >(CanvasSize.x));
    }
}

namespace Creators
{
    template< typename Func, typename... Args >
    tgui::Canvas::Ptr CreateCanvas(CanvasUpdater& UpdatePool, const tgui::Layout2d& Position, 
        const tgui::Layout2d& Size, Func&& Function, Args&&... Arguments)
    {
        tgui::Canvas::Ptr Canvas = tgui::Canvas::create();
        Canvas->setPosition(Position);
        Canvas->setSize(Size);
        UpdatePool.add({Canvas, std::bind(Function, Canvas, Arguments...)});
        return Canvas;
    }

    template< typename Func, typename... Args >
    tgui::Scrollbar::Ptr CreateScrollbar(const tgui::Layout2d& Position, const tgui::Layout2d& Size, 
        unsigned int MaxValue, unsigned int ViewportSize, Func&& Function, Args&&... Arguments)
    {
        tgui::Scrollbar::Ptr ScrollBar = tgui::Scrollbar::create();
        ScrollBar->setPosition(Position);
        ScrollBar->setSize(Size);
        ScrollBar->setMaximum(MaxValue);
        ScrollBar->setViewportSize(ViewportSize);
        ScrollBar->setValue(0);
        ScrollBar->connect(Signals::Scrollbar::ValueChanged, Function, Arguments...);
        return ScrollBar;
    }

    template< typename Func, typename... Args >
    tgui::Group::Ptr CreateScrollableCanvas(CanvasUpdater& UpdatePool, const tgui::Layout2d& Position, 
        const tgui::Layout2d& Size, const sf::Vector2u& WholeSize, Func&& DrawFunction, Args&&... Arguments)
    {
        tgui::Group::Ptr BoxLayout = tgui::BoxLayout::create();
        BoxLayout->setPosition(Position);
        BoxLayout->setSize(Size);

        tgui::Canvas::Ptr Canvas = CreateCanvas
        (
            UpdatePool, {0.f, 0.f},
            BoxLayout->getSize() - sf::Vector2f(20.f, 20.f),
            DrawFunction, Arguments...
        );
        BoxLayout->add(Canvas);
        sf::Vector2f CanvasSize = Canvas->getSize();

        tgui::Scrollbar::Ptr VerticalScroll = CreateScrollbar
        (
            {CanvasSize.x , 0.f}, {20.f, CanvasSize.y},
            WholeSize.y, static_cast< uint >(CanvasSize.y), 
            Handlers::HandleVScrollbar, 
            Canvas, VerticalScroll
        );
        BoxLayout->add(VerticalScroll);

        tgui::Scrollbar::Ptr HorizontalScroll = CreateScrollbar
        (
            {0.f , CanvasSize.y}, {CanvasSize.x, 20.f},
            WholeSize.x, static_cast< uint >(CanvasSize.x),
            Handlers::HandleHScrollbar, 
            Canvas, HorizontalScroll
        );
        BoxLayout->add(HorizontalScroll);

        BoxLayout->connect
        (
            Signals::Widget::SizeChanged, 
            Handlers::ScaleScrollableCanvas,
            BoxLayout, Canvas, VerticalScroll, 
            HorizontalScroll
        );
        return BoxLayout;
    }
}

void update(tgui::Canvas::Ptr& Canvas)
{
    sf::RectangleShape Shape;
    Shape.setPosition(500.f, 500.f);
    Shape.setSize({90.f, 65.f});
    Canvas->clear();
    Canvas->draw(Shape);
    Canvas->display();
}

void test(tgui::Gui& GUI, CanvasUpdater& UpdatePool)
{
    tgui::Group::Ptr MapCanvas = Creators::CreateScrollableCanvas
    (
        UpdatePool, {0, 0},
        {"100%", "100%"},
        {2000, 2000}, update
    );
    GUI.add(MapCanvas);
}

int main()
{
    sf::Event E;
    sf::RenderWindow Window(sf::VideoMode(1280, 768), "Test");
    tgui::Gui AppGui(Window);
    CanvasUpdater UpdatePool([&Window]{return Window.isOpen();});
    test(AppGui, UpdatePool);
    
    while(Window.isOpen())
    {
        while (Window.pollEvent(E))
        {
            if(E.type == sf::Event::Closed)
                Window.close();
            else if(E.type == sf::Event::Resized)
            {
                Window.setView(sf::View(sf::FloatRect(0.f, 0.f, 
                static_cast<float>(E.size.width), static_cast<float>(E.size.height))));
                AppGui.setView(Window.getView());
            }
            AppGui.handleEvent(E);
        }
        Window.clear({31, 33, 42});
        UpdatePool.Update();
        AppGui.draw();
        Window.display();
    }
    UpdatePool.RemoveOrphans();

    return 0;
}