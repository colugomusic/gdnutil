#pragma once

namespace gdn {

#define STRING(x) inline constexpr auto x{#x}

namespace signal {

STRING(about_to_show);
STRING(button_down);
STRING(button_pressed);
STRING(color_changed);
STRING(confirmed);
STRING(custom_action);
STRING(dir_selected);
STRING(draw);
STRING(file_selected);
STRING(files_dropped);
STRING(files_selected);
STRING(focus_entered);
STRING(focus_exited);
STRING(gui_input);
STRING(id_pressed);
STRING(item_activated);
STRING(item_edited);
STRING(item_rect_changed);
STRING(item_selected);
STRING(minimum_size_changed);
STRING(mouse_entered);
STRING(mouse_exited);
STRING(pressed);
STRING(popup_hide);
STRING(resized);
STRING(sort_children);
STRING(text_changed);
STRING(text_entered);
STRING(timeout);
STRING(tree_exiting);
STRING(toggled);
STRING(tween_all_completed);
STRING(tween_step);
STRING(value_changed);
STRING(value_changing);
STRING(visibility_changed);

} // signal

namespace slot {

STRING(grab_focus);
STRING(update);

} // slot

#undef STRING
} // gdn