#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>

#define WIN_X    800
#define WIN_Y    500

int readCSV();
void draw_callback();
void RedgeRegression();

char *iName = "SinData.csv";
char *oName = "SinData_Val.csv";

int dataNum;
double x[100], y[100];

#define DIMENSION 9
#define ALPHA exp(-10.0); // パラメータα

double w[DIMENSION+1]; // 学習パラメータ
double estimation; // 予測値

int main(int argc, char **argv)
{
    if (readCSV ()) {
        printf ("ファイルを読み込めません\n");
        return -1;
    }
    // リッジ回帰を求める
    RedgeRegression();
    
    GtkWidget *window;
    GtkWidget *drawing_area;
    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW(window), "sin(x)+e");
    gtk_widget_set_size_request (window, WIN_X, WIN_Y);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    drawing_area = gtk_drawing_area_new ();
    gtk_container_add (GTK_CONTAINER(window), drawing_area);
    g_signal_connect (drawing_area, "draw", G_CALLBACK(draw_callback), NULL);
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

void draw_callback(GtkWidget *widget, cairo_t *cr)
{
    int Origin[] = {50, WIN_Y/2};
    float lineWidth = 2.0;
    int num;
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    cairo_set_line_width (cr, lineWidth);
    cairo_move_to (cr, Origin[0], Origin[1]);
    cairo_line_to (cr, Origin[0]+WIN_X-100, Origin[1]);
    cairo_stroke (cr);
    cairo_move_to (cr, Origin[0], Origin[1]+200);
    cairo_line_to (cr, Origin[0], Origin[1]-200);
    cairo_stroke (cr);
    cairo_set_font_size (cr, 20.0);
    cairo_move_to       (cr, Origin[0]+WIN_X-100-150, Origin[1]+30);
    cairo_show_text     (cr, "X");
    cairo_set_font_size (cr, 20.0);
    cairo_move_to       (cr, 20, Origin[1]-200+20);
    cairo_show_text     (cr, "Y");
    cairo_set_source_rgba (cr, 0.0, 0.0, 1.0, 1.0);
    cairo_set_line_width (cr, lineWidth);
    
    for (num = 1; num < dataNum; num++) {
        cairo_move_to (cr, Origin[0]+x[num]*600+5, Origin[1]-y[num]*150);
        cairo_arc (cr, Origin[0]+x[num]*600, Origin[1]-y[num]*150, 5.0, 0.0, 2.0*3.14);
        cairo_stroke (cr);
    }
    
    // リッジ回帰の描画
    for (double Xval = 0.0; Xval < 1.0; Xval+=1/600.) {
        estimation = 0;
        for (int i = 0; i <= DIMENSION; i++) {
            estimation += w[i] * pow (Xval, i);
        }
        cairo_set_source_rgba (cr, 1.0, 0.0, 1.0, 1.0);
        
        cairo_move_to (cr, Origin[0]+Xval*600, Origin[1]-estimation*150);
        cairo_arc (cr, Origin[0]+Xval*600, Origin[1]-estimation*150, 1.0, 0.0, 2.0*3.14);
        cairo_stroke (cr);
        // y=sin(Xval)を描画
        cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, 1.0);
        cairo_move_to (cr, Origin[0]+Xval*600, Origin[1]-sin(Xval*2*3.14)*150);
        cairo_arc (cr, Origin[0]+Xval*600, Origin[1]-sin(Xval*2*3.14)*150, 1.0, 0.0, 2.0*3.14);
        cairo_stroke (cr);
    }
}

int readCSV () {
    FILE *fp;
    int cnt;
    int i, j;
    char fbuf[256];
    char iStr[256];
    char seps[]=",";
    char *token;
    char allData[4][40];
    
    fp = fopen(iName, "r");
    if(fp == NULL){
        printf("%sファイルがひらけません\n", iName);
        return FALSE;
    }
    dataNum = 0;
    while (fgets(fbuf,256,fp)) {
        j = 0;
        for (i = 0; i < strlen(fbuf); i++) {
            if (fbuf[i] == ',' && fbuf[i-1] == ',') {
                iStr[j] = ' ';
                j++;
                iStr[j] = fbuf[i];
                j++;
            } else {
                iStr[j] = fbuf[i];
                j++;
            }
        }
        
        token = strtok(iStr, seps);
        cnt = 0;
        while (token != NULL) {
            sscanf(token, "%s", allData[cnt]);
            token = strtok(NULL, seps);
            cnt ++;
        }
        
        x[dataNum] = atof(allData[0]);
        y[dataNum] = atof(allData[3]);
        
        dataNum++;
    }
    
    fclose (fp);
    fp = fopen(oName, "w");
    fprintf(fp, "X,Y\n");
    for (i = 1; i < dataNum; i++) {
        fprintf(fp, "%lf,%lf\n", x[i], y[i]);
    }
    fclose (fp);
    return 0;
}

// リッジ回帰を求める
void RedgeRegression() {
    
    // 行列の要素を求め代入
        // 拡大係数行列の正方行列 X
    double X[DIMENSION+1][DIMENSION+1];
    for (int i=0; i<=DIMENSION; i++) {
        for (int j=0; j<=DIMENSION; j++) {
            X[i][j] = pow(x[i+1], j);
        }
    }

    double X_T[DIMENSION+1][DIMENSION+1]; // Xの転置
    for (int i=0; i<=DIMENSION; i++) {
        for (int j=0; j<=DIMENSION; j++) {
            X_T[i][j] = X[j][i];
        }
    }
    
    double X_TX[DIMENSION+1][DIMENSION+1]; // Xの転置とXの積
    for (int i=0; i<=DIMENSION; i++) {
        for (int j=0; j<=DIMENSION; j++) {
            double bij = 0;
            for (int k=0; k<=DIMENSION; k++) {
                bij += X_T[i][k] * X[k][j];
            }
            X_TX[i][j] = bij;
        }
    }
    
        // 対角要素にαを加える
    double C[DIMENSION+1][DIMENSION+1]; // X_TXにALPHAと単位行列の積を足した行列(X^TX+αI)
    for (int i=0; i<=DIMENSION; i++) {
        for (int j=0; j<=DIMENSION; j++) {
            if (i==j) {
                C[i][j] = X_TX[i][j] + ALPHA;
            } else {
                C[i][j] = X_TX[i][j];
            }
        }
    }
    
    double X_Ty[DIMENSION+1][1]; // Xの転置とyの積
    for (int i=0; i<=DIMENSION; i++) {
        double bij = 0;
        for (int j=0; j<=DIMENSION; j++) {
            bij += X_T[i][j] * y[j+1];
        }
        X_Ty[i][0] = bij;
    }
    
    double ACM[DIMENSION+1][DIMENSION+2]; // 拡大係数行列(Augmented coefficient matrix)
    for (int i=0; i<=DIMENSION; i++) {
        for (int j=0; j<=DIMENSION; j++) {
            ACM[i][j] = C[i][j];
        }
        ACM[i][DIMENSION+1] = X_Ty[i][0];
    }

    // 掃き出し法を用いて連立方程式を解く
    for (int p=0; p<=DIMENSION; p++) {
        double a_pp = ACM[p][p];
        for (int i=p; i<=DIMENSION+1; i++) {
            ACM[p][i] /= a_pp;
        }
        for (int i=0; i<=DIMENSION; i++) {
            if (i!=p) {
                double a_ip = ACM[i][p];
                for (int j=p; j<=DIMENSION+1; j++) {
                    ACM[i][j] = ACM[i][j] - a_ip * ACM[p][j];
                }
            }
        }
    }
    
    for (int i=0; i<=DIMENSION; i++) {
        w[i] = ACM[i][DIMENSION+1];
    }
    
}
