#include "SplineViewerApp.hpp"

#include <fstream>

#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/components/LightComponent.hpp"

#include "CurveNode.hpp"
#include "PatchNode.hpp"

namespace GLOO {

SplineViewerApp::SplineViewerApp(const std::string& app_name,
                                 glm::ivec2 window_size,
                                 const std::string& filename)
    : Application(app_name, window_size), filename_(filename) {
}

void SplineViewerApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  LoadFile(filename_, root);

  auto camera_node = make_unique<ArcBallCameraNode>();
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.7f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(0.9f, 0.9f, 0.9f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 4.0f, 5.f));
  root.AddChild(std::move(point_light_node));
}

void SplineViewerApp::LoadFile(const std::string& filename, SceneNode& root) {
  std::fstream fs(GetAssetDir() + filename);
  if (!fs) {
    std::cerr << "ERROR: Unable to open file " + filename + "!" << std::endl;
    return;
  }

  std::string spline_type;
  std::getline(fs, spline_type);

  std::vector<glm::vec3> control_points;
  std::string line;
  for (size_t i = 0; std::getline(fs, line); i++) {
    std::stringstream ss(line);
    float x, y, z;
    ss >> x >> y >> z;
    control_points.push_back(glm::vec3(x, y, z));
  }

  // Convert spline_type from string to SplineBasis
  SplineBasis spline_basis;
  if (spline_type == "Bezier curve") {
    spline_basis = SplineBasis::Bezier;
  } else if (spline_type == "B-Spline curve") {
    spline_basis = SplineBasis::BSpline;
  } else {
    std::cerr << "ERROR: Spline basis type invalid" << std::endl;
    return;
  }
  // TODO: set up patch or curve nodes here.
  // The first line of the user-specified file is spline_type, and the specified
  // control points are in control_points, a std::vector of glm::vec3 objects.
  // Depending on the specified spline type, create the appropriate node(s)
  // parameterized by the control points.
  // auto curve_node = make_unique<CurveNode>(control_points, spline_basis);
  // root.AddChild(std::move(curve_node));

  if (control_points.size() > 4) {
    std::vector<glm::vec3> new_control_points;
    if (spline_basis == SplineBasis::Bezier) {
      // Each new control point shares one control point with the previous curve
      int num_curves = (control_points.size() + (control_points.size() / 4)) / 4;
      for (size_t i = 0; i < num_curves; i++) {
        new_control_points = {control_points[i*3], control_points[i*3+1], control_points[i*3+2], control_points[i*3+3]};
        auto curve_node = make_unique<CurveNode>(new_control_points, spline_basis);
        root.AddChild(std::move(curve_node));
      }
    } else {
      // Each new control point shares three control points with the previous curve
      for (size_t i = 0; i < control_points.size() - 3; i++) {
        new_control_points = {control_points[i], control_points[i+1], control_points[i+2], control_points[i+3]};
        auto curve_node = make_unique<CurveNode>(new_control_points, spline_basis);
        root.AddChild(std::move(curve_node));
      }
    }
  } else {
    auto curve_node = make_unique<CurveNode>(control_points, spline_basis);
    root.AddChild(std::move(curve_node));
  }
}
}  // namespace GLOO
