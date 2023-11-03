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
struct sphere_t {
    int index;
    point_t centre;
    float r;
    struct color_t color;
    };
struct hit_t {
    int hit;
    float dist;
    sphere_t obj;
    };
struct light_t {
    point_t pos;
    color_t col;
    float brightness;
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

point_t pointPlusVector(point_t a, vec_t b)
//adds vector b to point a
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

vec_t pointAdd(point_t a, point_t b)
//adds point a to point b
{
    vec_t result = {a.x+b.x, a.y+b.y, a.z+b.z};
    return result;
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

hit_t traceObj(point_t src, vec_t dir, sphere_t obj)
//determines if sphere obj is visible from source point src and the distance of the hit point
{
    hit_t h;
    vec_t L = pointSub(obj.centre, src);
    float tca = dot(L,dir);
    float d = sqrt(dot(L,L)-tca*tca);
    h.hit = 0;
    if(obj.r>d){
        float thc = sqrt(obj.r*obj.r - d*d);
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

hit_t traceScene(point_t src, vec_t direction, sphere_t *spheres)
//finds closest sphere at source point src
{
    hit_t closestHit;
            closestHit.hit = 0; 
            closestHit.dist = 9999;
            for(int i = 0; spheres[i].r>0; i++)
            {               
                hit_t h = traceObj(src, direction, spheres[i]);
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

color_t shadeObj(point_t src, float hDist, vec_t direction, sphere_t obj, sphere_t *spheres, light_t light, int repeat)
//returns dot product of N (surface normal at hit point P) and H (half vector between L and V)
{ 
    point_t P = pointPlusVector(src, vectorScale(hDist, normalise(direction))); //hit point
    hit_t shadowHit = traceScene(P, normalise(pointSub(light.pos, P)), spheres); //check for objects between hit point and light source
    if(shadowHit.hit) //cast shadows
        return {0,0,0}; 
    vec_t N = normalise(pointSub(obj.centre, P)); //surface normal 
    vec_t L = pointSub(P,light.pos); //light to hit point
    vec_t V = pointSub(P,src); //camera to hit point
    vec_t H; //half vector between L and V
    H.x = V.x+L.x;
    H.y = V.y+L.y;
    H.z = V.z+L.z;
    H = normalise(H);
    if(dot(N,H)<0)
        return {0,0,0};

    //reflections
    color_t RColor = {1,1,1};
    if (repeat == 0){
    vec_t Cv = vectorScale(dot(V,N), N);
    vec_t Rv = vecSub(vectorScale(2,Cv), V);
    hit_t Rh = traceScene(P,Rv,spheres);
    RColor = shadeObj(P, Rh.dist, Rv, Rh.obj, spheres, light, 1);
    }

    //base color of light at P: (light color * light brightness) / square of L's length
    color_t incidentLight;
    incidentLight.r = light.col.r*light.brightness/dot(L,L);
    if (incidentLight.r > 1)
        incidentLight.r = 1;
    incidentLight.g = light.col.g*light.brightness/dot(L,L);
    if (incidentLight.g > 1)
        incidentLight.g = 1;
    incidentLight.b = light.col.b*light.brightness/dot(L,L);
    if (incidentLight.b > 1)
        incidentLight.b = 1;
    
    //how close surface normal is to optimal reflection angle
    color_t reflectivity;
    reflectivity.r = dot(N,H)*obj.color.r;
    reflectivity.g = dot(N,H)*obj.color.g;
    reflectivity.b = dot(N,H)*obj.color.b;

    //product of all shading
    color_t resultLight;
    resultLight.r = incidentLight.r * reflectivity.r + RColor.r;
    resultLight.g = incidentLight.g * reflectivity.g + RColor.g;
    resultLight.b = incidentLight.b * reflectivity.b + RColor.b;
    return resultLight;
}

void drawScene(SDL_Renderer *renderer, int WindowWidth, int WindowHeight, point_t src, sphere_t *spheres, light_t light)
//draws closest sphere for each pixel in window
{
    float x;
    float y;
    vec_t direction;
    for(x=0; x<WindowWidth; x++)
    {
        for (y=0; y<WindowHeight; y++)
        {
            direction = normalise({x-WindowWidth/2,y-WindowHeight/2,focalLength});
            hit_t h = traceScene(src, direction, spheres);
            if (h.hit){
                color_t lightColor = shadeObj(src, h.dist, direction, h.obj, spheres, light, 0);
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
    SDL_SetWindowTitle(window, "sphere");
    // clear to background
    bool quit = false;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    point_t cameraPos = {0,0,0};
    sphere_t spheres[] = { {0,{-5,0,30},10,{255,0,0}},
                           {1,{0,10,40},12,{0,255,0}},
                           {2,{10,-10,30},8,{0,0,255}},
                           {3,{0,0,0},0,{0,0,0}}};  
    light_t light = {{5,0,10},{255,255,255},1};
    drawScene(renderer, WindowWidth, WindowHeight, cameraPos, spheres, light);
    int selectedSphere = 0;
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
            // select sphere with mouse
            case SDL_MOUSEBUTTONDOWN:
            {
                int x, y;
                SDL_GetMouseState( &x, &y );
                vec_t direction = normalise({(float)x-WindowWidth/2,(float)y-WindowHeight/2,focalLength});
                hit_t closestHit = traceScene(cameraPos, direction, spheres);
                //printf("%f, %f, %f \n", shadeObj(cameraPos, closestHit.dist, direction, closestHit.obj, light).r, shadeObj(cameraPos, closestHit.dist, direction, closestHit.obj, light).g, shadeObj(cameraPos, closestHit.dist, direction, closestHit.obj, light).b);
                selectedSphere = closestHit.obj.index;
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
                //move selected sphere
                case SDLK_d:
                    spheres[selectedSphere].centre.x++;
                    break;
                case SDLK_a:
                    spheres[selectedSphere].centre.x--;
                    break;
                case SDLK_w:
                    spheres[selectedSphere].centre.y--;
                    break;
                case SDLK_s:
                    spheres[selectedSphere].centre.y++;
                    break;
                case SDLK_q:
                    spheres[selectedSphere].centre.z--;
                    break;
                case SDLK_e:
                    spheres[selectedSphere].centre.z++;
                    break;
                case SDLK_z:
                    spheres[selectedSphere].r--;
                    break;
                case SDLK_c:
                    spheres[selectedSphere].r++;
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
                drawScene(renderer, WindowWidth, WindowHeight, cameraPos, spheres, light);
                // end of key process
            }   // end of keydown
            break;
            default:
                break;
            } // end of event switch
        }     // end of poll events
        // flip buffers
        SDL_RenderPresent(renderer);
        // wait 100 ms
        SDL_Delay(100);
    }
    // clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}