# gdnutil
GDNative utilities

## Typical GDNative class
```c++
class Example : public godot::Control
{
    GODOT_CLASS(Example, godot::Control);

public:

    static void _register_methods()
    {
        register_method("_on_mouse_entered", &Example::_on_mouse_entered);
        register_method("_on_mouse_exited", &Example::_on_mouse_exited);
        register_method("_on_custom_signal", &Example::_on_custom_signal);
        register_method("_on_thing_signal", &Example::_on_thing_signal);
        register_method("_on_widget_signal", &Example::_on_widget_signal);
        register_method("_ready", &Example::_ready);
        register_method("_gui_input", &Example::_gui_input);
        register_method("do_something", &Example::do_something);

        register_signal<Example>("custom_signal", "value", godot::Variant::INT);
    }

    void _init()
    {
        connect("mouse_entered", this, "_on_mouse_entered");
        connect("mouse_exited", this, "_on_mouse_exited");
        connect("custom_signal", this, "_on_custom_signal");
    }

    void attach_thing(SomeOtherThing* thing)
    {
        thing->connect("thing_signal", this, "_on_thing_signal");
    }

    void detach_thing()
    {
        thing->disconnect("thing_signal", this, "_on_thing_signal");
    }

    void set_widget(Widget* widget)
    {
        if (widget == widget_) return;

        if (widget_)
        {
            widget_->disconnect("widget_signal", this, "_on_widget_signal");
        }
        
        widget_ = widget;

        if (widget_)
        {
            widget_->connect("widget_signal", this, "_on_widget_signal");

            setup_widget();
        }
    }

private:

    void _ready()
    {
        autoloads_.Manager = cast_to<ManagerClass>(godot::Engine::get_singleton()->get_main_loop()->get_root()->get_node("Manager"));

        scene_.Title = cast_to<godot::Label>(get_node("Title"));
        scene_.Editor = cast_to<godot::LineEdit>(get_node("Editor"));
    }

    void _gui_input(godot::Ref<godot::InputEvent> event)
    {
        godot::Ref<godot::InputEventMouseButton> mb = event;

        if (mb.is_valid())
        {
            if (mb->get_button_index() == godot::GlobalConstants::BUTTON_LEFT)
            {
                if (mb->is_pressed())
                {
                    accept_event();
                }
            }
        }
    }

    void _on_mouse_entered() {}
    void _on_mouse_exited() {}
    void _on_custom_signal(int value) {}
    void _on_thing_signal() {}
    void _on_widget_signal() {}

    void do_something()
    {
        emit_signal("custom_signal", 123);
    }

    void setup_widget() {}

    struct
    {
        ManagerClass* Manager { nullptr };
    } autoloads_;

    struct
    {
        godot::Label* Title { nullptr };
        godot::LineEdit* Editor { nullptr };
    } scene_;

    Widget* widget_ { nullptr };
};
```
## Equivalent class written using gdnutil
```c++
class Example : public godot::Control
{
    GDN_CLASS(Example, godot::Control);

public:

    GDN_SIGNAL(custom_signal);

    static void _register_methods()
    {
        GDN_REG_SLOT(on_mouse_entered);
        GDN_REG_SLOT(on_mouse_exited);
        GDN_REG_SLOT(on_custom_signal);
        GDN_REG_SLOT(on_thing_signal);
        GDN_REG_SLOT(on_widget_signal);
        GDN_REG_METHOD(_ready);
        GDN_REG_METHOD(_gui_input);
        GDN_REG_METHOD(do_something);

        GDN_REG_SIGNAL((custom_signal, "value", godot::Variant::INT));

        Connections::SELF =
        {
            { mouse_entered, on_mouse_entered },
            { mouse_exited, on_mouse_exited },
            { custom_signal, on_custom_signal },
        };

        Connections::SOME_OTHER_THING =
        {
            { SomeOtherThing::thing_signal, on_thing_signal },
        };

        Connections::WIDGET =
        {
            { Widget::widget_signal, on_widget_signal },
        };
    }

    void _init()
    {
        Connections::SELF.connect(this).to(this);

        widget_.connector = [this](Widget* widget)
        {
            CONNECTIONS::WIDGET.connect(widget).to(this);

            setup_widget();
        };

        widget_.disconnector = [this](Widget* widget)
        {
            CONNECTIONS::WIDGET.disconnect(widget).from(this);
        };

        input_.gui.config.mb.left.on_pressed = [this](godot::Ref<godot::InputEventMouseButton> event)
        {
            accept_event();
        };
    }

    void attach_thing(SomeOtherThing* thing)
    {
        Connections::SOME_OTHER_THING.connect(thing).to(this);
    }

    void detach_thing(SomeOtherThing* thing)
    {
        Connections::SOME_OTHER_THING.disconnect(thing).from(this);
    }

    void set_widget(Widget* widget)
    {
        widget_ = widget;
    }

private:

    GDN_SLOT(on_mouse_entered, ()) {}
    GDN_SLOT(on_mouse_exited, ()) {}
    GDN_SLOT(on_custom_signal, (int value)) {}
    GDN_SLOT(on_thing_signal, ()) {}
    GDN_SLOT(on_widget_signal, ()) {}
    
    void _ready()
    {
        autoloads_.Manager = gdn::tree::get<ManagerClass>("Manager"));

        scene_.Title = gdn::tree::get<godot::Label>(this, "Title");
        scene_.Editor = gdn::tree::get<godot::LineEdit>(this, "Editor");
    }

    void _gui_input(godot::Ref<godot::InputEvent> event)
    {
        input_.gui(event);
    }

    void do_something()
    {
        emit_signal(custom_signal, 123);
    }

    void setup_widget() {}

    struct
    {
        ManagerClass* Manager { nullptr };
    } autoloads_;

    struct
    {
        godot::Label* Title { nullptr };
        godot::LineEdit* Editor { nullptr };
    } scene_;

    struct
    {
        gdn::InputHandler gui;
    } input_;

    gdn::Connectable<Widget> widget_;
};
```
