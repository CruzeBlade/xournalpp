#include "TextEditorContextMenu.h"

#include <iostream>
#include <string>

#include "control/Control.h"                // for Control
#include "gui/GladeSearchpath.h"            // for GladeSearchPath
#include "gui/PageView.h"                   // for PageView
#include "model/Text.h"                     // for Text
#include "view/overlays/TextEditionView.h"  // for TextEditionView

#include "TextEditor.h"  // for TextEditor

void changeFontInternal(GtkFontButton* src, TextEditorContextMenu* tecm) { tecm->changeFont(); }
void changeFtColorInternal(GtkColorButton* src, TextEditorContextMenu* tecm) { tecm->changeFtColor(); }
void changeBgColorInternal(GtkColorButton* src, TextEditorContextMenu* tecm) { tecm->changeBgColor(); }
void toggleAlignLeft(GtkButton* src, TextEditorContextMenu* tecm) { tecm->changeAlignment(TextAlignment::LEFT); }
void toggleAlignCenter(GtkButton* src, TextEditorContextMenu* tecm) { tecm->changeAlignment(TextAlignment::CENTER); }
void toggleAlignRight(GtkButton* src, TextEditorContextMenu* tecm) { tecm->changeAlignment(TextAlignment::RIGHT); }
gboolean drawFtColorIconInternal(GtkWidget* src, cairo_t* cr, TextEditorContextMenu* tecm) {
    return tecm->drawFtColorIcon(src, cr);
};
gboolean drawBgColorIconInternal(GtkWidget* src, cairo_t* cr, TextEditorContextMenu* tecm) {
    return tecm->drawBgColorIcon(src, cr);
};


TextEditorContextMenu::TextEditorContextMenu(Control* control, TextEditor* editor, XojPageView* pageView,
                                             GtkWidget* xournalWidget):
        control(control), editor(editor), pageView(pageView), xournalWidget(xournalWidget) {
    // Only for debugging
    std::cout << "TextEditorContextMenu created!" << std::endl;
    this->create();
}

TextEditorContextMenu::~TextEditorContextMenu() {
    std::cout << "Got here !" << std::endl;
    /*gtk_widget_destroy(GTK_WIDGET(this->fontBtn));
    gtk_widget_destroy(GTK_WIDGET(this->ftColorBtn));
    gtk_widget_destroy(GTK_WIDGET(this->bgColorBtn));
    gtk_widget_destroy(GTK_WIDGET(this->alignLeftTgl));
    gtk_widget_destroy(GTK_WIDGET(this->alignCenterTgl));
    gtk_widget_destroy(GTK_WIDGET(this->alignRightTgl));
    gtk_widget_destroy(GTK_WIDGET(this->contextMenu));*/
    std::cout << "TextEditorContextMenu destroyed!" << std::endl;
}

void TextEditorContextMenu::show() {
    this->reposition();
    gtk_widget_show_all(GTK_WIDGET(this->contextMenu));
    gtk_popover_popup(this->contextMenu);
    std::cout << "Popup menu should be shown" << std::endl;
}

void TextEditorContextMenu::hide() {
    gtk_popover_popdown(this->contextMenu);
    std::cout << "Popup menu should be hidden" << std::endl;
}

void TextEditorContextMenu::reposition() {
    int padding = xoj::view::TextEditionView::PADDING_IN_PIXELS;
    Range r = this->editor->getContentBoundingBox();
    GdkRectangle rect{this->pageView->getX() + int(r.getX() * this->pageView->getZoom()),
                      this->pageView->getY() + int(r.getY() * this->pageView->getZoom()) - padding,
                      int(r.getWidth() * this->pageView->getZoom()), int(r.getHeight() * this->pageView->getZoom())};
    gtk_popover_set_pointing_to(this->contextMenu, &rect);
}

void TextEditorContextMenu::create() {
    auto filepath = this->control->getGladeSearchPath()->findFile("", "textEditorContextMenu.glade");

    GtkBuilder* builder = gtk_builder_new();

    GError* err = NULL;
    if (gtk_builder_add_from_file(builder, filepath.u8string().c_str(), &err) == 0) {
        std::cout << err->message << std::endl;
    }

    this->contextMenu = GTK_POPOVER(gtk_builder_get_object(builder, "textEditorContextMenu"));
    gtk_popover_set_relative_to(this->contextMenu, this->xournalWidget);
    gtk_popover_set_constrain_to(this->contextMenu, GTK_POPOVER_CONSTRAINT_WINDOW);
    gtk_popover_set_modal(this->contextMenu, false);
    gtk_widget_set_can_focus(GTK_WIDGET(this->contextMenu), false);

    this->fontBtn = GTK_FONT_BUTTON(gtk_builder_get_object(builder, "btnFontChooser"));
    g_signal_connect(this->fontBtn, "font-set", G_CALLBACK(changeFontInternal), this);

    this->tglBoldBtn = GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoBold"));
    this->tglItalicBtn = GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoItalic"));
    this->tglUnderlineBtn = GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoUnderline"));
    this->expandTextDecoration = GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoExpand"));

    this->ftColorBtn = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "btnFontColor"));
    this->bgColorBtn = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "btnBgColor"));
    g_signal_connect(this->ftColorBtn, "color-set", G_CALLBACK(changeFtColorInternal), this);
    g_signal_connect(this->bgColorBtn, "color-set", G_CALLBACK(changeBgColorInternal), this);

    GdkRGBA color;

    GValue val = G_VALUE_INIT;
    g_value_init(&val, G_TYPE_POINTER);
    g_value_set_pointer(&val, &color);
    this->ftColorIcon = GTK_WIDGET(gtk_builder_get_object(builder, "imgFtColor"));
    g_object_set_property(G_OBJECT(this->ftColorBtn), "rgba", &val);
    g_signal_connect(this->ftColorIcon, "draw", G_CALLBACK(drawFtColorIconInternal), this);
    gtk_button_set_image(GTK_BUTTON(this->ftColorBtn), this->ftColorIcon);

    this->bgColorIcon = GTK_WIDGET(gtk_builder_get_object(builder, "imgBgColor"));
    g_object_set_property(G_OBJECT(this->ftColorBtn), "rgba", &val);
    g_signal_connect(this->bgColorIcon, "draw", G_CALLBACK(drawBgColorIconInternal), this);
    gtk_button_set_image(GTK_BUTTON(this->bgColorBtn), this->bgColorIcon);

    this->alignLeftTgl = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnAlignLeft"));
    this->alignCenterTgl = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnAlignCenter"));
    this->alignRightTgl = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnAlignRight"));
    g_signal_connect(this->alignLeftTgl, "released", G_CALLBACK(toggleAlignLeft), this);
    g_signal_connect(this->alignCenterTgl, "released", G_CALLBACK(toggleAlignCenter), this);
    g_signal_connect(this->alignRightTgl, "released", G_CALLBACK(toggleAlignRight), this);

    g_object_unref(G_OBJECT(builder));
}

void TextEditorContextMenu::changeFont() {
    PangoFontDescription* desc = gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(this->fontBtn));
    std::string fontDesc(pango_font_description_to_string(desc));
    std::string fontName(pango_font_description_get_family(desc));
    std::string fontSize(std::to_string(pango_font_description_get_size(desc)));
    std::cout << fontDesc << ":" << fontName << ":" << fontSize << std::endl;
    XojFont font;
    font = fontDesc;
    this->editor->setFont(font);
    pango_font_description_free(desc);
    gtk_widget_grab_focus(this->xournalWidget);
}

void TextEditorContextMenu::changeFtColor() {
    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(this->ftColorBtn), &color);
    this->editor->setColor(Color(uint8_t(color.red * 255.0), uint8_t(color.green * 255.0), uint8_t(color.blue * 255.0),
                                 uint8_t(color.alpha * 255.0)));
    gtk_widget_grab_focus(this->xournalWidget);
    std::cout << "New font color: (" << color.red << ";" << color.green << ";" << color.blue << ")" << std::endl;
}

void TextEditorContextMenu::changeBgColor() {
    GdkRGBA color;
    std::cout << "Got here a1!" << std::endl;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(this->bgColorBtn), &color);
    std::cout << "Got here a2!" << std::endl;
    this->editor->setBackgroundColor(color);
    std::cout << "Got here a3!" << std::endl;
    gtk_widget_grab_focus(this->xournalWidget);
    std::cout << "Got here a4!" << std::endl;
    auto selection = this->editor->getCurrentSelection().value_or(std::make_tuple(0, 0));
    std::cout << "New background color: (" << color.red << ";" << color.green << ";" << color.blue << ") from "
              << std::get<0>(selection) << ":" << std::get<1>(selection) << ";" << std::endl;
}

void TextEditorContextMenu::changeAlignment(TextAlignment align) {
    switch (align) {
        case TextAlignment::LEFT:
            gtk_toggle_button_set_active(this->alignLeftTgl, true);
            gtk_toggle_button_set_active(this->alignCenterTgl, false);
            gtk_toggle_button_set_active(this->alignRightTgl, false);
            break;
        case TextAlignment::CENTER:
            gtk_toggle_button_set_active(this->alignLeftTgl, false);
            gtk_toggle_button_set_active(this->alignCenterTgl, true);
            gtk_toggle_button_set_active(this->alignRightTgl, false);
            break;
        case TextAlignment::RIGHT:
            gtk_toggle_button_set_active(this->alignLeftTgl, false);
            gtk_toggle_button_set_active(this->alignCenterTgl, false);
            gtk_toggle_button_set_active(this->alignRightTgl, true);
            break;
        default:
            gtk_toggle_button_set_active(this->alignLeftTgl, false);
            gtk_toggle_button_set_active(this->alignCenterTgl, false);
            gtk_toggle_button_set_active(this->alignRightTgl, false);
            break;
    }
    this->editor->setTextAlignment(align);
    gtk_widget_grab_focus(this->xournalWidget);
}

gboolean TextEditorContextMenu::drawFtColorIcon(GtkWidget* src, cairo_t* cr) {
    guint width, height;
    GdkRGBA color;
    GtkStyleContext* context;
    context = gtk_widget_get_style_context(src);
    width = gtk_widget_get_allocated_width(src);
    height = gtk_widget_get_allocated_height(src);
    gtk_render_background(context, cr, 0, 0, width, height);

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(this->ftColorBtn), &color);

    // Draw font icon here
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 0, height - COLOR_BAR_HEIGHT, width, COLOR_BAR_HEIGHT);

    gdk_cairo_set_source_rgba(cr, &color);

    cairo_fill(cr);
    return FALSE;
}

gboolean TextEditorContextMenu::drawBgColorIcon(GtkWidget* src, cairo_t* cr) {
    guint width, height;
    GdkRGBA color;
    GtkStyleContext* context;
    context = gtk_widget_get_style_context(src);
    width = gtk_widget_get_allocated_width(src);
    height = gtk_widget_get_allocated_height(src);
    gtk_render_background(context, cr, 0, 0, width, height);

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(this->bgColorBtn), &color);

    // Draw font icon here
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 0, height - COLOR_BAR_HEIGHT, width, COLOR_BAR_HEIGHT);

    gdk_cairo_set_source_rgba(cr, &color);

    cairo_fill(cr);
    return FALSE;
}
