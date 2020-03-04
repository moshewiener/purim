#include <purim_api.h>

typedef struct css_provider_data { 
    int colour;
    int backcolour;
    GtkCssProvider *cssBtn;
} css_provider_data;

#define MAX_CSS_PROVIDERS 32

static gint cssProividersNum = 0;
static css_provider_data css_table[MAX_CSS_PROVIDERS];

/****************************************************************/
void init_css_table ( void )
{
    gint index;
    for (index=0; index < MAX_CSS_PROVIDERS; index++)
    {
        css_table[index].cssBtn = NULL;
        cssProividersNum = 0;
    }
}

/***************************************************************/
GtkCssProvider *set_css_provider( gint colour, gint backcolour)
{
    gint index;
    gchar css_string[128];
    GtkCssProvider *cssBtn;
    
    for (index=0; index < cssProividersNum; index++)
    {
        if (css_table[index].cssBtn != NULL)
            if (css_table[index].colour == colour)
               if (css_table[index].backcolour == backcolour)
                   return css_table[index].cssBtn;
    }
    // No matching css provider was found, and no more room for new providers
    if (cssProividersNum >= MAX_CSS_PROVIDERS)
        return NULL;

    // Assign a new css provider for this new colours combination to our widget
    cssBtn = gtk_css_provider_new();
    css_table[cssProividersNum].cssBtn = cssBtn;
    sprintf(css_string,
            "* { background: #%06X; border: #%06X; border-style: none; color: #%06X; }",
            backcolour, backcolour, colour);
    gtk_css_provider_load_from_data( cssBtn, css_string, -1, NULL );
    cssProividersNum++;
    
    return cssBtn;
}

/****************************************************************/
void css_set(GtkCssProvider *cssProvider, GtkWidget *g_widget)
{
    GtkStyleContext *context;
    
    context = gtk_widget_get_style_context(g_widget);
    
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_save(context);
}
