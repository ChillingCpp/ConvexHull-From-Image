#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

#define maxPolygonVertices 10

bool checkcolor(sf::Color color)
{
	return (color.r >= 0 || color.g >= 0 || color.b >= 0) && (color.a >= 150);
}
double cross(const sf::Vector2f &O, const sf::Vector2f &A, const sf::Vector2f &B)
{
	return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}
float disttoline(const sf::Vector2f &a, const sf::Vector2f &b, const sf::Vector2f &c)
{
	const sf::Vector2f bc = c - b;
	return std::abs(cross(a, b, c)) / std::hypot(bc.x, bc.y);
}
int linesimplycation(std::vector<bool> &mask, std::vector<sf::Vector2f> &hull, float tolerance)
{
	int first = 0;
	int second = 1;
	int third = 2;
	int pointCount = 0;
	for (int i = 0; i < hull.size() - 2; i++)
	{
		float dist = disttoline(hull[third], hull[first], hull[second]);
		if (dist <= tolerance)
		{
			mask[third] = false;
			third++;
		}
		else
		{
			first = second;
			second = third;
			third++;
			pointCount++;
		}
	}
	return pointCount;
}
std::vector<sf::Vector2f> linesimplycation(std::vector<sf::Vector2f> &hull, float tolerance = 0.f)
{
	std::vector<bool> mask(hull.size(), true);
	int pointCount = hull.size();

	while (pointCount > maxPolygonVertices)
	{
		pointCount = linesimplycation(mask, hull, tolerance);
		tolerance += 0.5f;
	}

	std::vector<sf::Vector2f> points;
	for (int i = 0; i < mask.size(); i++)
	{
		if (mask[i])
			points.push_back(hull[i]);
	}

	return points;
}
bool is_left_turn(const sf::Vector2f &p, const sf::Vector2f &q, const sf::Vector2f &r)
{

	return (q.x - p.x) * (r.y - p.y) > (r.x - p.x) * (q.y - p.y); 
	// (q[0] - p[0])*(r[1] - p[1]) > (r[0] - p[0])*(q[1] - p[1])
}
std::vector<sf::Vector2f> convex_hull(std::vector<sf::Vector2f> &P)
{
	size_t n = P.size(), k = 0;
	if (n <= 3)
		return P;
	// data structures
	std::vector<sf::Vector2f> upper;
	std::vector<sf::Vector2f> lower;

	// // Endpoints
	sf::Vector2f p0 = P[0];
	sf::Vector2f pn = P.back();

	// // Initial line(s) of separation
	double ku = (pn.y - p0.y) / (pn.x - p0.x + 1e-12);
	double nu = p0.y - ku * p0.x;
	double kl = ku;
	double nl = nu;

	// Add left endpoint
	upper.push_back(p0);
	lower.push_back(p0);

	// // Construct the middle of the upper and lower hull sections
	for (int j = 1; j < n - 1; j++)
	{
		sf::Vector2f p = P[j];
		if (p.y > ku * p.x + nu)
		{
			while (upper.size() > 1 && is_left_turn(upper[upper.size() - 2], upper[upper.size() - 1], p))
			{
				upper.pop_back();
			}
			upper.push_back(p);
			ku = (pn.y - p0.y) / (pn.x - p0.x + 1e-12);
			nu = p0.y - ku * p0.x;
		}
		// p[0] == p.x
		// p[1] == p.y
		else if (p.y < kl * p.x + nl)
		{
			while (lower.size() > 1 && !(is_left_turn(lower[lower.size() -2], lower[lower.size() -1], p)))
				lower.pop_back();

			lower.push_back(p);
			kl = (pn.y - p.y) / (pn.x - p.x + 1e-12);
			nl = p.y - kl * p.x;
		}
	}

	// // Add right endpoint (only once due to merging that follows)
	while (upper.size() > 1 && is_left_turn(upper[upper.size() - 2], upper[upper.size() - 1], pn))
		upper.pop_back();

	while (lower.size() > 1 && !(is_left_turn(lower[lower.size() -2], lower[lower.size() -1], pn)))
		lower.pop_back();

	upper.push_back(pn);

	// # Reverse lower hull section
	std::reverse(lower.begin(), lower.end());

	// # Merge hull sections
	upper.insert(upper.end(), lower.begin(), lower.end());
	for (int i = 0; i < upper.size(); i++)
	{
		upper[i] = sf::Vector2f(upper[i].y, upper[i].x);
	}

	return upper;
};
void TransformHull(std::vector<sf::Vector2f> &H, const sf::Transform &transform)
{
	for (int i = 0; i < H.size(); i++)
	{
		H[i] = transform * H[i];
	}
}
int main()
{

	sf::RenderWindow window(sf::VideoMode(900, 700), "Convex Hull generator");

	sf::Texture t;
	t.loadFromFile("image/linux.png");
	sf::Image image = t.copyToImage();

	sf::Sprite sprite;
	sprite.setTexture(t);

	std::vector<sf::Vector2f> points;

	sf::Vector2u size(t.getSize());

	// transform image pixels to coordinates positions;
	for (int i = 0; i < size.y; i++)
	{
		for (int j = 0; j < size.x; j++)
		{
			if (checkcolor(image.getPixel(j, i)))
			{
				points.push_back(sf::Vector2f(i, j));
			}
		}
	}
	sf::Vector2f pos(window.getSize() / (size_t)2);

	sprite.setOrigin(sf::Vector2f(t.getSize() / (size_t)2));
	sprite.setPosition(pos);
	sprite.rotate(10);
	sprite.setScale(1.2f, 1.2f);

	// those points have already been sorted
	std::vector<sf::Vector2f> H = convex_hull(points);

	H = linesimplycation(H); // reduce convexhull vertices for better performance

	TransformHull(H, sprite.getTransform());

	sf::VertexArray vertices(sf::LineStrip, H.size() + 1);

	for (int i = 0; i < H.size(); i++)
	{
		vertices[i] = sf::Vertex(H[i], sf::Color::Red);
	}
	vertices[vertices.getVertexCount() - 1] = sf::Vertex(H[0], sf::Color::Red);

	sf::Clock clock;

	while (window.isOpen())
	{
		float dt = clock.restart().asSeconds();
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();
		}
		window.clear(sf::Color(100, 100, 100));
		window.draw(sprite);
		window.draw(vertices);
		window.display();
	}

	return 0;
}