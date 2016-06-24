// Ben Chapman-Kish
// 2016-06-07
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;

#define PI 3.1415926535898

int framerate, width, height, ballCount, redirectDistOffset;
bool edgeCollisionBounce, edgeDoubleWrap;
float ballFriction, cursorRedirectFactor, gravityRedirectFactor;
float redirectDistIntensity, redirectDistExpansion, redirectDistCurve;
float redirectSizeIntensity, redirectSizeExpansion, redirectSizeCurve;

class Ball {
private:
	sf::Vector2f pos;
	sf::CircleShape shape;
	sf::Color col;
	sf::Texture *tex;
	bool image;
public:
	float x, y, rot, xvel, yvel, rotvel, rad;
	sf::Vertex line1[2], line2[2], line3[2];
	Ball(float, float, float, float, sf::Color);
	Ball(float, float, float, float, sf::Texture*);
	
	void setpos(float, float);
	void setvel(float, float);
	void move();
	void warp(bool);
	void wallbounce();
	void redirect(float, float, float);
	void gravity(vector<Ball> &, int);
	//void collide(vector<Ball> &, int);
	
	void draw(sf::RenderWindow &);
	virtual ~Ball();
};

Ball::Ball(float xv, float yv, float rv, float r, sf::Color c) {
	xvel = xv;
	yvel = yv;
	rotvel = rv;
	rad = r;
	col = c;
	tex = 0;
	image = false;
	x = width / 2.0;
	y = height / 2.0;
	rot = 0;
	
	shape = sf::CircleShape(rad);
	shape.setOrigin(rad, rad);
	shape.setPosition(x, y);
	shape.setFillColor(col);
	pos = shape.getPosition();
}

Ball::Ball(float xv, float yv, float rv, float r, sf::Texture *t) {
	xvel = xv;
	yvel = yv;
	rotvel = rv;
	rad = r;
	tex = t;
	image = true;
	x = width / 2.0;
	y = height / 2.0;
	rot = 0;
	
	shape = sf::CircleShape(rad);
	shape.setOrigin(rad, rad);
	shape.setPosition(x, y);
	shape.setTexture(tex, true);
	shape.setOutlineThickness(1);
	shape.setOutlineColor(sf::Color(0, 0, 0));
	pos = shape.getPosition();
}

void Ball::move() {
	x += xvel;
	y += yvel;
	//cout << rotvel << endl << rot << endl << endl;
	rot += rotvel;
	while (rot < 0) rot += 360;
	while (rot >= 360) rot -= 360;
	xvel *= ballFriction;
	yvel *= ballFriction;
	rotvel *= ballFriction;
	shape.setPosition(x, y);
	shape.setRotation(rot);
}

void Ball::setpos(float nx, float ny) {
	x = nx;
	y = ny;
	shape.setPosition(x, y);
}

void Ball::setvel(float nx, float ny) {
	xvel = nx;
	yvel = nx;
}

void Ball::warp(bool full) {
	pos = shape.getPosition();
	
	float offset = 0;
	if (full) offset = rad;
	
	if (x - offset > width)
		x = 0 - offset;
	if (y - offset > height)
		y = 0 - offset;
	if (x + offset < 0)
		x = width + offset;
	if (y + offset < 0)
		y = height + offset;
	
	shape.setPosition(x, y);
}

void Ball::wallbounce() {
	pos = shape.getPosition();
	
	if (x + rad > width) {
		x = width - rad;
		xvel *= -1;
	} else if (x - rad < 0) {
		x = rad;
		xvel *= -1;
	}
	if (y + rad > height) {
		y = height - rad;
		yvel *= -1;
	} else if (y - rad < 0) {
		y = rad;
		yvel *= -1;
	}
	
	shape.setPosition(x, y);
}

float ft(float x, int offset, int intensity, float expansion, float curve) {
	if (x >= offset)
		return intensity / pow((pow((x - offset) / expansion, 2) + 1), curve);
	else
		return (cos(PI * x / offset) - 1) / -2;
}

void Ball::redirect(float dx, float dy, float factor) {
	float dist = sqrt(pow(x - dx, 2) + pow(y - dy, 2));
	float dmag;
	if (factor > 0)
		dmag = ft(dist, 0, redirectDistIntensity, redirectDistExpansion, redirectDistCurve);
	else
		dmag = ft(dist, redirectDistOffset, redirectDistIntensity, redirectDistExpansion, redirectDistCurve);
	float smag = ft(rad, 0, redirectSizeIntensity, redirectSizeExpansion, redirectSizeCurve);
	
	float angle = PI / 2;
	angle = atan2(y - dy, x - dx);
	
	float velangle = PI / 2;
	if (xvel == yvel) {
		if (xvel < 0)
			velangle -= PI;
	} else {
		velangle = atan2(yvel, xvel);
	}
	//~ if (velangle < 0)
	//~ velangle += 2 * PI;
	
	float rotmag = pow(abs(velangle - (factor < 0 ? angle + PI : angle)), 2) * dmag * smag / 50;
	if (velangle - (factor < 0 ? angle + PI : angle) > PI) {
		rotvel += rotmag;
	} else if (velangle - (factor < 0 ? angle + PI : angle) < PI) {
		rotvel -= rotmag;
	}
	
	line1[0] = sf::Vertex(sf::Vector2f(x, y), sf::Color(0, 255, 0));
	line1[1] = sf::Vertex(sf::Vector2f(x + cos(velangle) * 200, y + sin(velangle) * 200), sf::Color(0, 255, 0));
	
	line2[0] = sf::Vertex(sf::Vector2f(x, y), sf::Color(255, 100, 100));
	line2[1] = sf::Vertex(sf::Vector2f(x + cos((factor < 0 ? angle + PI : angle)) * 200, y + sin((factor < 0 ? angle + PI : angle)) * 200), sf::Color(255, 0, 0));
	
	
	cout << "velangle: " << velangle << endl;
	cout << "rotvel: " << rotvel << endl;
	cout << "rot: " << rot << endl;
	cout << "angle: " << angle << endl;
	cout << "rotvel * angle: " << rotvel * angle << endl;
	cout << "rotmag: " << rotmag << endl;
	cout << endl;
	
	xvel += cos(angle) * dmag * smag * factor;
	yvel += sin(angle) * dmag * smag * factor;
}

void ripple(vector<Ball> &balls, int x, int y, int factor) {
	for (int i = balls.size() - 1; i >= 0; i--)
		balls.at(i).redirect(x, y, factor);
}

void Ball::gravity(vector<Ball> &balls, int position) {
	for (int i = balls.size() - 1; i >= 0; i--) {
		if (i == position) continue;
		balls.at(i).redirect(x, y, -gravityRedirectFactor);
	}
}

/*void Ball::collide(vector<Ball> &balls, int position) {
	for (int i = balls.size() - position - 1; i >= 0; i--) {
 Ball sb = balls.at(i);
 if (pos.x + rad > sb.x - sb.rad &&
 pos.x - rad < sb.x + sb.rad &&
 pos.y + rad > sb.y - sb.rad &&
 pos.y - rad < sb.y + sb.rad)
 {
 if (pow(pos.x - sb.x, 2) + pow(pos.y - sb.y, 2) < pow(rad + sb.rad, 2)) {
 // Collision occured
 float colAngle = atan2(pos.y - sb.y, pos.x - sb.x);
 float initAngle1 = atan2(pos.y, pos.x);
 float initVel1 = sqrt(pow(pos.x, 2) + pow(pos.y, 2));
 float initAngle2 = atan2(sb.y, sb.x);
 float initVel2 = sqrt(pow(sb.x, 2) + pow(sb.y, 2));
 
 float v1x = initVel1 * cos(initAngle1 - colAngle);
 float v1y = initVel1 * sin(initAngle1 - colAngle);
 float v2x = initVel2 * cos(initAngle2 - colAngle);
 float v2y = initVel2 * sin(initAngle2 - colAngle);
 
 float f1x = v1x(rad - sb.rad) + 2*sb.rad*v2x;
 float f2x = v2x(rad - sb.rad) + 2*sb.rad*v1x;
 
 this->move();
 sb.move();
 
 }
 }
	}
 }*/


void Ball::draw(sf::RenderWindow &window) {
	window.draw(shape);
	if (edgeDoubleWrap && !edgeCollisionBounce) {
		float nx(x), ny(y);
		bool atEdge = false;
		
		if (x + rad > width) {
			nx -= width;
			atEdge = true;
		} else if (x - rad < 0) {
			nx += width;
			atEdge = true;
		}
		if (y + rad > height) {
			ny -= height;
			atEdge = true;
		} else if (y - rad < 0) {
			ny += height;
			atEdge = true;
		}
		if (atEdge) {
			sf::CircleShape copy = sf::CircleShape(rad);
			if (image) {
				copy.setTexture(tex, true);
				copy.setOutlineThickness(1);
				copy.setOutlineColor(sf::Color(0, 0, 0));
			} else {
				copy.setFillColor(col);
			}
			copy.setOrigin(rad, rad);
			copy.setPosition(nx, ny);
			copy.setRotation(rot);
			
			window.draw(copy);
		}
	}
}

Ball::~Ball() {
}

int randrange(int low, int high) {
	int range = high - low + 1;
	return low + (rand() / (RAND_MAX + 1.0)) * range;
}


int main() {
	srand(static_cast<unsigned>(time(0)));
	
	ballFriction = 0.98;
	edgeCollisionBounce = false;
	edgeDoubleWrap = true;
	cursorRedirectFactor = 5;
	gravityRedirectFactor = 0;
	
	redirectDistOffset = 200;
	redirectDistIntensity = 1;
	redirectDistExpansion = 10;
	redirectDistCurve = 0.3;
	
	redirectSizeIntensity = 1;
	redirectSizeExpansion = 10;
	redirectSizeCurve = 0.3;
	
	ballCount = 1;
	framerate = 60;
	width = 1280;
	height = 800;
	
	sf::RenderWindow window(sf::VideoMode(width, height), "I have balls.");
	window.setFramerateLimit(framerate);
	
	sf::Texture tex;
	if (!tex.loadFromFile("image.jpg")) return EXIT_FAILURE;
	tex.setSmooth(true);
	
	bool windowFocus = true;
	
	vector <Ball> balls;
	
	for (int i = 0; i < ballCount; i++)
		balls.push_back(Ball(randrange(-20, 20), randrange(-20, 20), randrange(-50, 50), randrange(20, 100), &tex));
	//balls.push_back(Ball(randrange(-20, 20), randrange(-20, 20), randrange(20, 100), sf::Color(randrange(30, 255), randrange(30, 255), randrange(30, 255))));
	
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::LostFocus:
					windowFocus = false;
					break;
				case sf::Event::GainedFocus:
					windowFocus = true;
					break;
				case sf::Event::KeyPressed:
					switch (event.key.code) {
						case sf::Keyboard::Escape:
							window.close();
							break;
						default:
							break;
					}
					break;
				case sf::Event::MouseButtonPressed:
					if (event.mouseButton.button == sf::Mouse::Middle) {
						balls.insert(balls.begin(), Ball(randrange(-20, 20), randrange(-20, 20), randrange(-50, 50), randrange(20, 100), &tex));
						balls.at(0).setpos(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
					}
					/*switch (event.mouseButton.button) {
						case sf::Mouse::Left:
					 ripple(balls, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, 1);
					 break;
						case sf::Mouse::Right:
					 ripple(balls, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, -1);
					 break;
						case sf::Mouse::Middle:
					 
					 break;
						default:
					 break;
					 }*/
					break;
				default:
					break;
			}
		}
		if (!windowFocus) continue;
		
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			ripple(balls, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, 2*cursorRedirectFactor);
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
			ripple(balls, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, -cursorRedirectFactor);
		
		
		for (int i = balls.size() - 1; i >= 0; i--) {
			if (edgeCollisionBounce) {
				balls.at(i).wallbounce();
			} else {
				balls.at(i).warp(!edgeDoubleWrap);
			}
			balls.at(i).move();
		}
		
		for (int i = balls.size() - 1; i >= 0; i--) {
			//balls.at(i).gravity(balls, i);
			//balls.at(i).collide(balls, i);
		}
		
		window.clear(sf::Color::Black);
		for (int i = balls.size() - 1; i >= 0; i--) {
			balls.at(i).draw(window);
			window.draw(balls.at(i).line1, 2, sf::Lines);
			window.draw(balls.at(i).line2, 2, sf::Lines);
		}
		window.display();
	}
	
	return EXIT_SUCCESS;
}

