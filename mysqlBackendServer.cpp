// mysqlBackendServer.cpp : 애플리케이션의 진입점을 정의합니다.
//

#include "mysqlBackendServer.h"
#include <httplib.h>
#include <mysqlx/xdevapi.h>
#include <nlohmann/json.hpp>
#include <iostream>

using namespace httplib;
using namespace std;
using json = nlohmann::json;
using namespace mysqlx;

int main()
{
	Session session("localhost", 33060, "root", "1234");	

	Server svr;

	Schema db = session.getSchema("test");
	Table table = db.getTable("blog");

	svr.Options("/create", [&](const Request& req, Response& res) {
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Methods", "*");
		res.set_header("Access-Control-Allow-Headers", "Content-Type");
		res.set_content("", "text/plain");
	});

	svr.Post("/create", [&](const Request& req,  Response& res) {

		json response = {
			{"state", 200}
		};

		try {
			json j = json::parse(req.body);

			const std::string title = j["title"];
			const std::string content = j["content"];

			table.insert("title", "content").values(title, content).execute();
			cout << "post ok" << endl;
		}	
		catch (const mysqlx::Error& err) {
			response = {
			{"state", 500}
			};
			cout << "post error" << endl;
		}

		res.set_content(response.dump(), "application/json");
	});

	svr.Get("/list", [&](const Request& req,  Response& res) {
		res.set_header("Access-Control-Allow-Origin", "*");

		RowResult result = table.select("title", "content").execute();

		json response = json::array();

		cout << "list 접속" << "\n";

		for (const Row& row : result) {
			json item = {
				{"title", row[0].get<std::string>()},
				{"content", row[1].get<std::string>()}
			};

			response.push_back(item);
		}


		res.set_content(response.dump(),"application/json");
	});

	svr.listen("localhost", 8080);

	return 0;
}
