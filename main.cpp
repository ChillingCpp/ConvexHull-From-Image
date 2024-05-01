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
std::vector<sf::Vector2f> convex_hull(std::vector<sf::Vector2f> &P)
{
	size_t n = P.size(), k = 0;
	if (n <= 3)
		return P; 
	std::vector<sf::Vector2f> H(2 * n);
	// build lower hull;
	for (size_t i = 0; i < n; ++i)
	{
		while (k >= 2 && cross(H[k - 2], H[k - 1], P[i]) <= 0)
			k--;
		H[k++] = P[i];
	}
	// build upper hull;
	for (size_t i = n - 1, t = k + 1; i > 0; --i)
	{
		while (k >= t && cross(H[k - 2], H[k - 1], P[i - 1]) <= 0)
			k--;
		H[k++] = P[i - 1];
	}
	H.resize(k - 1);
	for (int i = 0; i < H.size(); i++)
	{
		H[i] = sf::Vector2f(H[i].y, H[i].x);
	}
	return H;
};
void TransformHull(std::vector<sf::Vector2f> &H, const sf::Transform &transform)
{
	for (int i = 0; i < H.size(); i++)
	{
		H[i] = transform*H[i];
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
	sf::Vector2f pos(window.getSize()/(size_t)2);
	
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
	vertices[vertices.getVertexCount() - 1] = H[0];

	while (window.isOpen())
	{

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