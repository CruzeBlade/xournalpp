#include "LinkDialog.h"

#include "control/Control.h"      // for Control
#include "gui/GladeSearchpath.h"  // for GladeSearchPath

void okButtonClicked(GtkButton* btn, LinkDialog* dialog) { dialog->okButtonPressed(btn); }
void cancelButtonClicked(GtkButton* btn, LinkDialog* dialog) { dialog->cancelButtonPressed(btn); }
void textChangedClb(GtkTextBuffer* buffer, LinkDialog* dialog) { dialog->textChanged(buffer); }
void layoutToogledLeft(GtkToggleButton* source, LinkDialog* dialog) { dialog->layoutToggled(Layout::LEFT); };
void layoutToogledCenter(GtkToggleButton* source, LinkDialog* dialog) { dialog->layoutToggled(Layout::CENTER); };
void layoutToogledRight(GtkToggleButton* source, LinkDialog* dialog) { dialog->layoutToggled(Layout::RIGHT); };

LinkDialog::LinkDialog(Control* control) {
    auto filepath = control->getGladeSearchPath()->findFile("", "linkDialog.glade");

    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, filepath.u8string().c_str(), NULL);

    this->linkDialog = GTK_DIALOG(gtk_builder_get_object(builder, "linkDialog"));
    this->textInput = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "inpLinkEditText"));
    this->urlInput = GTK_ENTRY(gtk_builder_get_object(builder, "inpLinkEditURL"));
    this->okButton = GTK_BUTTON(gtk_builder_get_object(builder, "btnLinkEditOk"));
    this->cancelButton = GTK_BUTTON(gtk_builder_get_object(builder, "btnLinkEditCancel"));

    this->fontChooser = GTK_FONT_CHOOSER(gtk_builder_get_object(builder, "btnFontChooser"));

    this->layoutLeft = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnLeftLayout"));
    this->layoutCenter = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnCenterLayout"));
    this->layoutRight = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnRightLayout"));

    this->linkTypeChooser = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "cbLinkPrefix"));

    g_signal_connect(G_OBJECT(okButton), "clicked", G_CALLBACK(okButtonClicked), this);
    g_signal_connect(G_OBJECT(cancelButton), "clicked", G_CALLBACK(cancelButtonClicked), this);
    g_signal_connect(G_OBJECT(gtk_text_view_get_buffer(this->textInput)), "changed", G_CALLBACK(textChangedClb), this);

    g_signal_connect(G_OBJECT(layoutLeft), "released", G_CALLBACK(layoutToogledLeft), this);
    g_signal_connect(G_OBJECT(layoutCenter), "released", G_CALLBACK(layoutToogledCenter), this);
    g_signal_connect(G_OBJECT(layoutRight), "released", G_CALLBACK(layoutToogledRight), this);
}

LinkDialog::~LinkDialog() { gtk_widget_destroy(GTK_WIDGET(linkDialog)); }

void LinkDialog::preset(std::string text, std::string url, Layout layout) {
    GtkTextBuffer* textBuffer = gtk_text_view_get_buffer(this->textInput);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(textBuffer, &start, &end);
    gtk_text_buffer_insert(textBuffer, &start, text.c_str(), text.length());
    gtk_text_view_set_buffer(this->textInput, textBuffer);
    gtk_entry_set_text(this->urlInput, url.c_str());
    this->layoutToggled(layout);
}

int LinkDialog::show() { return gtk_dialog_run(GTK_DIALOG(this->linkDialog)); }

std::string LinkDialog::getText() { return this->linkText; }

std::string LinkDialog::getURL() { return this->linkURL; }

void LinkDialog::okButtonPressed(GtkButton* btn) {
    std::cout << "OK Button pressed" << std::endl;
    GtkTextBuffer* text = gtk_text_view_get_buffer(this->textInput);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(text, &start, &end);
    this->linkText = std::string(gtk_text_buffer_get_text(text, &start, &end, TRUE));
    std::cout << "  Link Text: " << this->linkText << std::endl;
    if (!this->isTextValid(this->linkText)) {
        this->linkText = "Link";
    }

    this->linkURL = std::string(gtk_entry_get_text(this->urlInput));
    std::cout << "  Link URL: " << this->linkURL << std::endl;
    if (!this->isUrlValid(this->linkURL)) {
        this->linkURL = "xournalpp.github.io";
    }

    gtk_dialog_response(linkDialog, LinkDialog::SUCCESS);
}

void LinkDialog::cancelButtonPressed(GtkButton* btn) {
    std::cout << "Cancel Button pressed" << std::endl;
    gtk_dialog_response(linkDialog, LinkDialog::CANCEL);
}


bool LinkDialog::isTextValid(std::string text) {
    if (text.empty()) {
        return false;
    }

    int nBlankChars = 0;
    for (char c: text) {
        if (std::isblank(c)) {
            nBlankChars++;
        }
    }

    if (text.length() == nBlankChars) {
        return false;
    }

    return true;
}

bool LinkDialog::isUrlValid(std::string url) {
    if (url.empty()) {
        return false;
    }
    return true;
}


void LinkDialog::textChanged(GtkTextBuffer* buffer) {
    gint lot = gtk_text_buffer_get_line_count(buffer);
    int width, height;
    gtk_window_get_size(GTK_WINDOW(this->linkDialog), &width, &height);
    height = DEFAULT_HEIGHT + (std::max(0, (lot - INITIAL_NUMBER_OF_LINES)) * ADDITIONAL_HEIGHT_PER_LINE);
    height = std::min(height, MAX_HEIGHT);
    gtk_window_resize(GTK_WINDOW(this->linkDialog), width, height);
}

void LinkDialog::layoutToggled(Layout layout) {
    this->layout = layout;
    if (layout == Layout::LEFT) {
        gtk_toggle_button_set_active(this->layoutLeft, true);
        gtk_toggle_button_set_active(this->layoutCenter, false);
        gtk_toggle_button_set_active(this->layoutRight, false);
    } else if (layout == Layout::CENTER) {
        gtk_toggle_button_set_active(this->layoutLeft, false);
        gtk_toggle_button_set_active(this->layoutCenter, true);
        gtk_toggle_button_set_active(this->layoutRight, false);
    } else if (layout == Layout::RIGHT) {
        gtk_toggle_button_set_active(this->layoutLeft, false);
        gtk_toggle_button_set_active(this->layoutCenter, false);
        gtk_toggle_button_set_active(this->layoutRight, true);
    }
}

Layout LinkDialog::getLayout() { return this->layout; }
