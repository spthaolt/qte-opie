#include <qimage.h>
#include <qcolor.h>

static inline int blendComponent( int v, int av, int s, int as )
{
    //shadow gets a color inversely proportional to the
    //alpha value
    s = (s*(255-as))/255;
    //then do standard blending
    return as*s + av*v -(av*as*s)/255;  
}
    
static inline QRgb blendShade( QRgb v, QRgb s )
{
    //pick a number: shadow is 1/3 of object
    int as = qAlpha(s)/3; 
    int av = qAlpha(v);
    if ( as == 0 || av == 255 )
	return v;

    int a = as + av -(as*av)/255;

    
    int r = blendComponent( qRed(v),av, qRed(s), as)/a;
    int g = blendComponent( qGreen(v),av, qGreen(s), as)/a;
    int b = blendComponent( qBlue(v),av, qBlue(s), as)/a;

    return qRgba(r,g,b,a);
}

int main( int*, char**)
{
    QImage *img;

    img = new QImage( "in.png" );
    int w,h;
    int y;
    img->setAlphaBuffer( TRUE );
    *img = img->convertDepth( 32 );
    w = img->width();
    h = img->height();
#if 0
    for ( y = 0; y < h; y ++ ) {
	uint *line = (uint*)img->scanLine( y );
	for ( int x = 0; x < w; x++ ) {
	    uint pixel = line[x];
	    int r = qRed(pixel);
	    int g = qGreen(pixel);
	    int b = qBlue(pixel);
	    int min = QMIN( r, QMIN( g, b ) );
	    int max = QMAX( r, QMAX( g, b ) );
	    r -= min;
	    g -= min;
	    b -= min;
	    if ( max !=min ) {
		r = (r*255)/(max-min);
		g = (g*255)/(max-min);
		b = (b*255)/(max-min);
	    }
	    int a = 255-min;
	    a -=  (max-min)/3; //hack more transparency for colors.
	    line[x] = qRgba( r, g, b, a );
	}
    }
#endif    
    *img = img->smoothScale( w/2, h/2 );

    qDebug( "saving out.png");
    img->save( "out.png", "PNG" );
    
    w = img->width();
    h = img->height();
    
    QImage *img2 = new QImage( w, h, 32 );
    img2->setAlphaBuffer( TRUE );
    for ( y = 0; y < h; y++ ) {
	for ( int x = 0; x < w; x++ ) {
	    QRgb shader = img->pixel( x, y );

	    int as = qAlpha(shader)/3;

	    int r = (qRed(shader)*(255-as))/255;
	    int g = (qGreen(shader)*(255-as))/255;
	    int b = (qBlue(shader)*(255-as))/255;

	    img2->setPixel( x, y, qRgba(r,g,b,as) ); 
	}
    }

    img2->save( "outshade.png", "PNG" );

}
