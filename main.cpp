#ifdef WIN32
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>
#include <random>
#include <cmath>
#include <math.h>
#include <SDL_image.h>
const int focalLength = 500;

void putPixel(SDL_Renderer *renderer, float x, float y, unsigned char r, unsigned char g, unsigned char b)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawPoint(renderer, x, y);
}

struct point_t {
    float x;
    float y;
    float z;
    };
struct vec_t {
    float x;
    float y;
    float z;
    };
struct color_t {
    float r;
    float g;
    float b;
    };

struct obj_t;

struct hit_t {
    int hit;
    float dist;
    obj_t *obj;
    };
struct light_t {
    point_t pos;
    color_t col;
    float brightness;
    };
struct obj_t {
    int index;

    //trace functions for different object types sphere and triangle
    hit_t (*traceObj)(point_t src, vec_t dir, obj_t *obj);
    vec_t (*surfaceNormal)(obj_t obj, point_t p);
    
    //centre and radius for sphere object type
    point_t centre;
    float r;

    //points for triangle object type
    point_t p1, p2, p3;

    //shading values
    color_t color;
    float Ka;
    float Ks;
    float Kd;
    float reflect;
    };

float dot(vec_t a, vec_t b)
//dot product operation
{
    float result = a.x*b.x+a.y*b.y+a.z*b.z;
    return(result);
}

vec_t vectorScale(float x, vec_t v)
//multiplies vector v by float x
{
    v.x *= x;
    v.y *= x;
    v.z *= x;
    return v;
}

point_t pointPlusVector(point_t p, vec_t v)
//adds vector b to point a
{
    p.x += v.x;
    p.y += v.y;
    p.z += v.z;
    return p;
}

vec_t pointSub(point_t a, point_t b)
//subtracts point b from point a
{
    vec_t result = {a.x-b.x, a.y-b.y, a.z-b.z};
    return result;
}

vec_t vecSub(vec_t a, vec_t b)
//subtracts vector b from vector a
{
    vec_t result = {a.x-b.x, a.y-b.y, a.z-b.z};
    return result;
}

vec_t normalise(vec_t v)
//normalises vector v
{
    float length = sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
    vec_t result = {v.x/length, v.y/length, v.z/length};
    return result;
}

color_t addColor(color_t a, color_t b)
//adds two colors
{
    a.r += b.r;
    a.g += b.g;
    a.b += b.b;
    return a;
}

color_t scaleColor(float x, color_t c)
//multiplies color c by float x
{
    c.r *= x;
    c.g *= x;
    c.b *= x;
    return c;
}

vec_t crossProd(vec_t a, vec_t b)
//cross product
{   
    vec_t v;
    v.x = a.y*b.z - a.z*b.y;
    v.y = -(a.x*b.z - a.z*b.x);
    v.z = a.x*b.y - a.y*b.x;
    return v;
}

vec_t surfNormSphere(obj_t obj, point_t p)
//surface normal of sphere
{
    return(normalise(pointSub(obj.centre, p)));
}

vec_t surfNormTriangle(obj_t obj, point_t p)
//surface normal of triangle
{
    p = {0,0,0};
    vec_t p1p2 = pointSub(obj.p2,obj.p1);
    vec_t p1p3 = pointSub(obj.p3,obj.p1);
    return(normalise(crossProd(p1p2,p1p3)));
}

hit_t traceSphere(point_t src, vec_t dir, obj_t *obj)
//determines if sphere obj is visible from source point src and finds the distance of the hit point
{
    hit_t h;
    vec_t L = pointSub(obj->centre, src);
    float tca = dot(L,dir);
    float d = sqrt(dot(L,L)-tca*tca);
    h.hit = 0;
    if(obj->r>d){
        float thc = sqrt(obj->r*obj->r - d*d);
        h.dist = tca - thc;
        h.obj = obj;
        if(h.dist>0.001)
            h.hit = 1;
        else{
            h.dist = tca + thc;
            if(h.dist>0.001)
            h.hit = 1;
        }
    }
    return h;
}

hit_t traceTriangle(point_t src, vec_t dir, obj_t *obj)
//determines if triangle obj is visible from source point src and finds the distance of the hit point
//https://www.icloud.com/keynote/0-QS-DHZa5bzEnBi-e9YNATHQ#02.BasicSurfaces
{
    hit_t h;
    //get surface normal of triangle plane
    vec_t p1p2 = pointSub(obj->p2,obj->p1);
    vec_t p1p3 = pointSub(obj->p3,obj->p1);
    vec_t N = normalise(crossProd(p1p2,p1p3));

    float d = dot(N,pointSub(obj->p1,{0,0,0}));   
    h.hit = 0;
    
    h.dist=(d-dot(N,pointSub(src,{0,0,0})))/dot(N,dir); 
    if(h.dist<0.001){
        h.hit = 0;
        return h;
    }

    point_t P = pointPlusVector(src, vectorScale(h.dist,dir));

    vec_t AP = pointSub(P,obj->p1);
    vec_t AB = p1p2;
    vec_t AC = p1p3;

    float v = (AP.y - (AP.x * AB.y)/AB.x)/(AC.y-((AC.x*AB.y)/AB.x));
    float u = (AP.x - v * AC.x)/AB.x;
    if (u>0 && v >0){
        if(u+v<=1){
        h.hit = 1;
        //printf("%f, %f, %f \n", P.x, P.y, P.z);
        //printf("%f, %f, \n", u, v);
        h.obj = obj;
        }
    }
    return h;
}

hit_t traceScene(point_t src, vec_t direction, obj_t *objects)
//finds closest object at source point src
{
    hit_t closestHit;
            closestHit.hit = 0; 
            closestHit.dist = 9999;
            for(int i = 0; objects[i].r>0; i++)
            {               
                hit_t h = objects[i].traceObj(src, direction, &objects[i]);
                if(h.hit){
                    if (h.dist < closestHit.dist){
                        closestHit.obj = h.obj;
                        closestHit.dist = h.dist;
                        closestHit.hit = 1;
                    }
                } 
            }
    return(closestHit);
}

color_t shadeObj(point_t src, float hDist, vec_t direction, obj_t *obj, obj_t *objects, light_t light, int repeat)
//returns dot product of N (surface normal at hit point P) and H (half vector between L and V)
{ 
    point_t P = pointPlusVector(src, vectorScale(hDist, normalise(direction))); //hit point
    hit_t shadowHit = traceScene(P, normalise(pointSub(light.pos, P)), objects); //check for objects between hit point and light source
    if(shadowHit.hit) //cast shadows
        return {0,0,0};
    vec_t N = obj->surfaceNormal(*obj, P); //surface normal 
    vec_t L = pointSub(P,light.pos); //light to hit point
    vec_t V = pointSub(P,src); //camera to hit point
    vec_t H; //half vector between L and V
    H.x = V.x+L.x;
    H.y = V.y+L.y;
    H.z = V.z+L.z;
    H = normalise(H);
    if(dot(N,H)<0)
        return {0,0,0};

    color_t resultColor;
    resultColor.r=0;
	resultColor.g=0;
	resultColor.b=0;

    //ambient: object color * ambient of object
    color_t ambientLight = scaleColor(obj->Ka, obj->color);

    //diffuse: color of light at P: ((light color * light brightness) / square of L's length) * diffuse of object
    color_t diffuseLight;
    diffuseLight = scaleColor(obj->Kd, scaleColor((light.brightness/dot(L,L)), light.col));

    //specular: how close surface normal is to optimal reflection angle * specular of obj
    color_t specHighlight;
    specHighlight = scaleColor(obj->Ks, scaleColor(pow(dot(N,H),2), obj->color));

    //reflections
    color_t RColor = {0,0,0};
    if (repeat > 0){
        vec_t Vv = normalise(pointSub(src,P)); //hit point to camera
        vec_t Cv = vectorScale(dot(Vv,N), N);
        vec_t Rv = vecSub(vectorScale(2,Cv), Vv);
        hit_t Rh = traceScene(P,Rv,objects);
    if (Rh.hit)
        RColor = scaleColor(obj->reflect, shadeObj(P, Rh.dist, Rv, Rh.obj, objects, light, repeat-1));
    }

    //product of all shading
    resultColor.r = ambientLight.r + diffuseLight.r + specHighlight.r + RColor.r;
    resultColor.g = ambientLight.g + diffuseLight.g + specHighlight.g + RColor.g;
    resultColor.b = ambientLight.b + diffuseLight.b + specHighlight.b + RColor.b;
    return resultColor;
}

void drawScene(SDL_Renderer *renderer, int WindowWidth, int WindowHeight, point_t src, obj_t *objects, light_t light)
//draws closest object for each pixel in window
{
    float x;
    float y;
    vec_t direction;
    for(x=0; x<WindowWidth; x++)
    {
        for (y=0; y<WindowHeight; y++)
        {
            direction = normalise({x-WindowWidth/2,y-WindowHeight/2,focalLength});
            hit_t h = traceScene(src, direction, objects);
            if (h.hit){
                color_t lightColor = shadeObj(src, h.dist, direction, h.obj, objects, light, 1);
                putPixel(renderer,x,y, lightColor.r, lightColor.g, lightColor.b);
            }
        }
    }
}

auto main() -> int
{
    constexpr int WindowWidth = 700;
    constexpr int WindowHeight = 700;
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Surface* pScreenShot = SDL_CreateRGBSurface(0, WindowWidth, WindowHeight, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    std::random_device rd;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WindowWidth, WindowHeight, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "render");
    // clear to background
    bool quit = false;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    point_t cameraPos = {0,0,0};
    obj_t objects[] = { {0,traceSphere,surfNormSphere,{-6,-4,30},10,{0,0,0},{0,0,0},{0,0,0},{255,25,25}, 0.1, 0.8, 0.8, 0.5},
                        {1,traceSphere,surfNormSphere,{12,10,25},5,{0,0,0},{0,0,0},{0,0,0},{25,255,25}, 0.1, 0.8, 0.8, 0.5},
                        {2,traceSphere,surfNormSphere,{10,-10,30},8,{0,0,0},{0,0,0},{0,0,0},{25,25,255}, 0.1, 0.8, 0.8, 0.5},
                        {3,traceTriangle,surfNormTriangle,{-5,0,0},1,{-5,5,30},{10,5,30},{5,15,30},{155,155,155}, 0.1, 0.8, 0.8, 0.5},
                        {4,traceTriangle,surfNormTriangle,{-5,0,0},1,{-5,5,20},{10,5,30},{0,15,30},{155,155,155}, 0.1, 0.8, 0.8, 0.5},
                        {5,NULL,NULL,{0,0,0},0,{0,0,0},{0,0,0},{0,0,0},{0,0,0}, 0, 0, 0, 0}};
    light_t light = {{-5,0,10},{255,255,255},1};
    drawScene(renderer, WindowWidth, WindowHeight, cameraPos, objects, light);
    int selectedObject = 0;
    //render
    while (!quit)
    {

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            // this is the window x being clicked.
            case SDL_QUIT:
                quit = true;
                break;
            // select object with mouse
            case SDL_MOUSEBUTTONDOWN:
            {
                int x, y;
                SDL_GetMouseState( &x, &y );
                vec_t direction = normalise({(float)x-WindowWidth/2,(float)y-WindowHeight/2,focalLength});
                hit_t closestHit = traceScene(cameraPos, direction, objects);
                if (closestHit.hit)
                {
                    selectedObject = closestHit.obj->index;
                    printf("%d \n", selectedObject);
                }
                break;
            }
            // now we look for a keydown event
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
                {
                // if it's the escape key quit
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                // if p screenshot
                case SDLK_p:
                    if(pScreenShot)
                    {
                        // Read the pixels from the current render target and save them onto the surface
                        SDL_RenderReadPixels(renderer, NULL, SDL_GetWindowPixelFormat(window), pScreenShot->pixels, pScreenShot->pitch);

                        // Create the bmp screenshot file
                        SDL_SaveBMP(pScreenShot, "Screenshot.bmp");
                        // Destroy the screenshot surface
                        SDL_FreeSurface(pScreenShot);
                    }
                    break;
                //move selected object
                case SDLK_d:
                    objects[selectedObject].centre.x++;
                    objects[selectedObject].p1.x++;
                    objects[selectedObject].p2.x++;
                    objects[selectedObject].p3.x++;
                    break;
                case SDLK_a:
                    objects[selectedObject].centre.x--;
                    objects[selectedObject].p1.x--;
                    objects[selectedObject].p2.x--;
                    objects[selectedObject].p3.x--;
                    break;
                case SDLK_w:
                    objects[selectedObject].centre.y--;
                    objects[selectedObject].p1.y--;
                    objects[selectedObject].p2.y--;
                    objects[selectedObject].p3.y--;
                    break;
                case SDLK_s:
                    objects[selectedObject].centre.y++;
                    objects[selectedObject].p1.y++;
                    objects[selectedObject].p2.y++;
                    objects[selectedObject].p3.y++;
                    break;
                case SDLK_q:
                    objects[selectedObject].centre.z--;
                    objects[selectedObject].p1.z--;
                    objects[selectedObject].p2.z--;
                    objects[selectedObject].p3.z--;
                    break;
                case SDLK_e:
                    objects[selectedObject].centre.z++;
                    objects[selectedObject].p1.z++;
                    objects[selectedObject].p2.z++;
                    objects[selectedObject].p3.z++;
                    break;
                case SDLK_z:
                    objects[selectedObject].r--;
                    break;
                case SDLK_c:
                    objects[selectedObject].r++;
                    break; 
                //move camera
                case SDLK_UP:
                    cameraPos.y--;
                    break;
                case SDLK_DOWN:
                    cameraPos.y++;
                    break;
                case SDLK_RIGHT:
                    cameraPos.x++;
                    break;
                case SDLK_LEFT:
                    cameraPos.x--;
                    break;
                case SDLK_COMMA:
                    cameraPos.z--;
                    break;
                case SDLK_PERIOD:
                    cameraPos.z++;
                    break;
                default:
                    break;
                } 
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderClear(renderer);
                drawScene(renderer, WindowWidth, WindowHeight, cameraPos, objects, light);
                // end of key process
            }   // end of keydown
            break;
            default:
                break;
            } // end of event switch
        }     // end of poll events
        // flip buffers
        SDL_RenderPresent(renderer);
    }
    // clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
