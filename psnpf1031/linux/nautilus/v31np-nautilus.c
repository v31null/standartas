#include <nautilus-extension.h>

#include <string.h>

#define V31NP_TYPE_MENU (v31np_menu_get_type())
G_DECLARE_FINAL_TYPE(V31npMenu, v31np_menu, V31NP, MENU, GObject)

struct _V31npMenu
{
    GObject parent;
};

static void v31np_menu_provider_init(NautilusMenuProviderInterface* iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(V31npMenu, v31np_menu, G_TYPE_OBJECT, G_TYPE_FLAG_FINAL,
    G_IMPLEMENT_INTERFACE_DYNAMIC(NAUTILUS_TYPE_MENU_PROVIDER, v31np_menu_provider_init))

static const char* const verbs[] = { "name", "here", "there", "therename", "allhere", "allthere" };

static gchar* helper(void)
{
    gchar* local = g_build_filename(g_get_home_dir(), ".local", "bin", "v31np-menu", NULL);
    if (g_file_test(local, G_FILE_TEST_IS_EXECUTABLE)) return local;
    g_free(local);
    return g_find_program_in_path("v31np-menu");
}

static void activate(NautilusMenuItem* item, gpointer data)
{
    const char* verb = data;
    GPtrArray* paths = g_object_get_data(G_OBJECT(item), "v31np-paths");
    gchar* program = helper();
    if (!program || !paths) return;
    GPtrArray* argv = g_ptr_array_new();
    g_ptr_array_add(argv, program);
    g_ptr_array_add(argv, (gpointer)verb);
    for (guint i = 0; i < paths->len; i++) g_ptr_array_add(argv, g_ptr_array_index(paths, i));
    g_ptr_array_add(argv, NULL);
    g_spawn_async(NULL, (gchar**)argv->pdata, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, NULL);
    g_ptr_array_free(argv, TRUE);
    g_free(program);
}

static NautilusMenuItem* entry(const char* id, const char* label, const char* verb, GPtrArray* paths)
{
    NautilusMenuItem* item = nautilus_menu_item_new(id, label, NULL, NULL);
    g_object_set_data_full(G_OBJECT(item), "v31np-paths", g_ptr_array_ref(paths), (GDestroyNotify)g_ptr_array_unref);
    g_signal_connect(item, "activate", G_CALLBACK(activate), (gpointer)verb);
    return item;
}

static gboolean packed(NautilusFileInfo* file)
{
    if (nautilus_file_info_is_mime_type(file, "application/x-v31np")) return TRUE;
    gchar* name = nautilus_file_info_get_name(file);
    const size_t n = name ? strlen(name) : 0;
    const gboolean match = n > 6 && g_ascii_strcasecmp(name + n - 6, ".v31np") == 0 && !nautilus_file_info_is_directory(file);
    g_free(name);
    return match;
}

static gchar* bare(const char* path)
{
    gchar* name = g_path_get_basename(path);
    const size_t n = strlen(name);
    if (n > 6 && g_ascii_strcasecmp(name + n - 6, ".v31np") == 0) name[n - 6] = '\0';
    return name;
}

static GList* items(NautilusMenuProvider* provider, GList* files)
{
    (void)provider;
    if (!files) return NULL;
    GPtrArray* paths = g_ptr_array_new_with_free_func(g_free);
    gboolean all = TRUE;
    for (GList* l = files; l; l = l->next)
    {
        NautilusFileInfo* file = l->data;
        GFile* location = nautilus_file_info_get_location(file);
        gchar* path = location ? g_file_get_path(location) : NULL;
        if (location) g_object_unref(location);
        if (!path)
        {
            g_ptr_array_unref(paths);
            return NULL;
        }
        g_ptr_array_add(paths, path);
        if (!packed(file)) all = FALSE;
    }
    GList* out = NULL;
    if (all && paths->len == 1)
    {
        gchar* name = bare(g_ptr_array_index(paths, 0));
        gchar* first = g_strconcat("Anfas alùz e íülíntù ", name, NULL);
        gchar* fourth = g_strconcat("Anfas alùz țùre e íülíntù ", name, NULL);
        out = g_list_append(out, entry("V31np::name", first, verbs[0], paths));
        out = g_list_append(out, entry("V31np::here", "Anfas alùz híre", verbs[1], paths));
        out = g_list_append(out, entry("V31np::there", "Anfas alùz țùre", verbs[2], paths));
        out = g_list_append(out, entry("V31np::therename", fourth, verbs[3], paths));
        g_free(first);
        g_free(fourth);
        g_free(name);
    }
    else if (all)
    {
        out = g_list_append(out, entry("V31np::allhere", "Anfas teí̈nt híre", verbs[4], paths));
        out = g_list_append(out, entry("V31np::allthere", "Anfas teí̈nt țùre", verbs[5], paths));
    }
    else out = g_list_append(out, entry("V31np::pack", "Baíbíz", "pack", paths));
    g_ptr_array_unref(paths);
    return out;
}

static void v31np_menu_provider_init(NautilusMenuProviderInterface* iface)
{
    iface->get_file_items = items;
}

static void v31np_menu_init(V31npMenu* self)
{
    (void)self;
}

static void v31np_menu_class_init(V31npMenuClass* klass)
{
    (void)klass;
}

static void v31np_menu_class_finalize(V31npMenuClass* klass)
{
    (void)klass;
}

static GType types[1];

void nautilus_module_initialize(GTypeModule* module)
{
    v31np_menu_register_type(module);
    types[0] = V31NP_TYPE_MENU;
}

void nautilus_module_shutdown(void)
{
}

void nautilus_module_list_types(const GType** list, int* count)
{
    *list = types;
    *count = G_N_ELEMENTS(types);
}
