#include <gtk/gtk.h>
#include <epoxy/gl.h>

static GtkWidget *gl_area = NULL;
static GLuint program_vbo;
static GLuint program;
GLint err;

static const GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f};

static const char *vertex_shader =
"#version 330\n"
"layout(location = 0) in vec4 position;\n"
"void main() {\n"
"gl_Position = position;\n"
"}";

static const char *fragment_shader =
"#version 330\n"
"out vec4 outputColor;\n"
"void main() {\n"
"float lerpVal = gl_FragCoord.y / 500.0f;\n"
"outputColor = mix(vec4(1.0f, 0.85f, 0.35f, 1.0f), vec4(0.2f, 0.2f, 0.2f, 1.0f), lerpVal);\n"
"}";

static gboolean
render(GtkGLArea *area,
       GdkGLContext *context)
{
    if (gtk_gl_area_get_error(area) != NULL)
        return FALSE;

    /* Clear the viewport */
    glClearColor(0.9, 0.9, 0.9, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, program_vbo);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // glBufferSubData(GL_ARRAY_BUFFER, 0, 2*sizeof(vertices), vertices);

    err = glGetError();
    printf("%d\n", err);
    /* Draw our object */
    glDrawArrays(GL_TRIANGLES, 0, 3);

    err = glGetError();
    printf("%d\n", err);

    /* We finished using the buffers and program */
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    /* Flush the contents of the pipeline */
    glFlush();

    return TRUE;
}

void unrealize(GtkWidget *widget)
{
    gtk_gl_area_make_current(GTK_GL_AREA(widget));

    if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
        return;
}

void realize(GtkWidget *widget)
{
    gtk_gl_area_make_current(GTK_GL_AREA(widget));
    if (gtk_gl_area_get_error (GTK_GL_AREA (widget)) != NULL)
        return;
    GLuint vbo, vao;
    int status;
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    program_vbo = vbo;

    GLuint v_shader, f_shader;

    v_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v_shader, 1, &vertex_shader, NULL);
    glCompileShader(v_shader);

    f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f_shader, 1, &fragment_shader, NULL);
    glCompileShader(f_shader);

    program = glCreateProgram();
    glAttachShader(program, f_shader);
    glAttachShader(program, v_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        int log_len;
        char *buffer;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

        buffer = g_malloc(log_len + 1);
        glGetProgramInfoLog(program, log_len, NULL, buffer);

        g_warning("Linking failure:\n%s", buffer);

        g_free(buffer);

        glDeleteProgram(program);
        program = 0;

        goto out;
    }

    glDetachShader(program, v_shader);
    glDetachShader(program, f_shader);

out:
    glDeleteShader(v_shader);
    glDeleteShader(f_shader);
}

void activate(GtkApplication *app)
{
    GtkWidget *window, *box;
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "OpenGL Area");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_window_destroy), NULL);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, FALSE);
    gtk_widget_set_margin_start(box, 0);
    gtk_widget_set_margin_end(box, 0);
    gtk_widget_set_margin_top(box, 0);
    gtk_widget_set_margin_bottom(box, 0);
    gtk_box_set_spacing(GTK_BOX(box), 0);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gl_area = gtk_gl_area_new();
    gtk_widget_set_hexpand(gl_area, TRUE);
    gtk_widget_set_vexpand(gl_area, TRUE);
    gtk_widget_set_size_request(gl_area, 100, 200);
    gtk_box_append(GTK_BOX(box), gl_area);

    /* We need to initialize and free GL resources, so we use
     * the realize and unrealize signals on the widget
     */
    g_signal_connect(gl_area, "realize", G_CALLBACK(realize), NULL);
    g_signal_connect(gl_area, "unrealize", G_CALLBACK(unrealize), NULL);

    /* The main "draw" call for GtkGLArea */
    g_signal_connect(gl_area, "render", G_CALLBACK(render), NULL);

    gtk_widget_show(window);
}

int main(int argc,
         char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
