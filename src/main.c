#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <cglm/cglm.h>
#include <time.h>

#include <pthread.h>

static GtkWidget *gl_area = NULL;
static GLuint program_vbo;
static GLuint program;
pthread_t thread_id;
static GLuint mvp_location;

static float angle=0.f;
static vec3 pos={0.f,0.f};
GLint err;



static const GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f};

void delay(int milli_seconds)
{
    // Converting time into milli_seconds


    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds);
}

void *rotate(){
    printf("here");
    delay(2000);
    while(1){
        delay(1000);
        if(angle==360.f)
        angle = 0.f;
        angle+=0.05f;

        gtk_widget_queue_draw (gl_area);
    }
}

static const char *vertex_shader =
"#version 330\n"
"layout(location = 0) in vec4 position;\n"
"uniform mat4 MVP;\n"
"void main() {\n"
"gl_Position = MVP * position;\n"
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
    mat4 m={0};
    vec3 v = {0.f, 0.f, 1.f};
    
    glm_rotate_make(m, angle, v);


    // for(int i=0;i<sizeof(mvp);i++){
    //     printf("%f\n", mvp[i]);
    // }

    /* Clear the viewport */
    delay(500);
    glClearColor(0.f, 0.f, 0.f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);

    glUniformMatrix4fv (mvp_location, 1, GL_FALSE, &m[0][0]);
    glBindBuffer(GL_ARRAY_BUFFER, program_vbo);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // glBufferSubData(GL_ARRAY_BUFFER, 0, 2*sizeof(vertices), vertices);

    
    /* Draw our object */
    glDrawArrays(GL_TRIANGLES, 0, 3);


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
    GLuint mvp = 0;
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

    mvp = glGetUniformLocation (program, "MVP");
    mvp_location = mvp;
    glDetachShader(program, v_shader);
    glDetachShader(program, f_shader);

out:
    glDeleteShader(v_shader);
    glDeleteShader(f_shader);

    
    pthread_create(&thread_id, NULL, rotate, NULL);

    
}

void activate(GtkApplication *app)
{
    GtkWidget *window, *box, *button;
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

    button = gtk_button_new_with_label ("Quit");
    gtk_widget_set_hexpand (button, TRUE);
    gtk_box_append (GTK_BOX (box), button);
    g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_destroy), window);

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
