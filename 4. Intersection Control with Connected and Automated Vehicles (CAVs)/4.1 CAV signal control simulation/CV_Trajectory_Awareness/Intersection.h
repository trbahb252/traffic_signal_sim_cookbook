//This class defines intersection attributes

class Intersection
{
	public:
	int ID;   //intersection ID
	int Laner_No; //Number of the lanes at the intersection
	int lane_phase_mapping[50];  //Maximum 50 lanes per intersection
	float inter_size;  //This is the radius of the intersection
	int DSRC_range[4];  //The DSRC range of each approach, mainly consider close space intersections, the range don't exceed the upstream intersection, with the sequence of the approach number
	vector<N_LaneNodes> MAP_Nodes;
};
