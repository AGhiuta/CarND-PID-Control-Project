#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

// for convenience
using json = nlohmann::json;
using namespace std;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main()
{
  uWS::Hub h;
  PID pid;

  double c_time = 0.0; // Current frame time
  double p_time = clock(); // Previous frame time 

  double Kp = 0.12, Ki = 0.01, Kd = 0.004;

  pid.Init(Kp, Ki, Kd);  // Init PID parameters

  h.onMessage([&pid, &c_time, &p_time](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data));
      if (s != "") {
        auto j = json::parse(s);
        string event = j[0].get<string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = stod(j[1]["cte"].get<string>());
          double speed = stod(j[1]["speed"].get<string>());
          double angle = stod(j[1]["steering_angle"].get<string>());
          double steer_value;

          c_time = clock();
          double dt = (c_time - p_time) / CLOCKS_PER_SEC;
          p_time = c_time;

          // Set throttle

          double thr = fabs(cte) > 0.25 ? 0.25 : 1.0;

          if (fabs(pid.p_error - cte) > 0.1 and fabs(pid.p_error - cte) <= 0.2){
            thr = 0.0;
		      } else if (fabs(pid.p_error - cte) > 0.1 and speed > 30){
            thr = -0.5; // Break!
          }

          pid.UpdateError(cte, dt);

          steer_value = -pid.TotalError();
          steer_value = fmax(-1.0, fmin(steer_value, 1.0));

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = thr;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);

          // DEBUG
          std::cout << "CTE: " << cte << " Steering Value: " << steer_value << std::endl;
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }  
    }
  });

   //We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  std::cout << "HELLO\n";
  h.run();
}
