#include "picCore.h"

namespace pic
{

tuple3i getClosestNBRCellIdcs(MacGrid const & grid, Vector3d const & worldSpacePos, int i, int j, int k){
	Vector3d const cellSpacePos = grid.cellSpacePos(worldSpacePos, i, j, k);
	return {
		cellSpacePos[0] < .5f ? i - 1 : i + 1,
		cellSpacePos[1] < .5f ? j - 1 : j + 1,
		cellSpacePos[2] < .5f ? k - 1 : k + 1
	};
}

tuple8i getStageredCellFaceNBRIdcs_u(MacGrid const & grid, int min_i, int min_j, int min_k, int max_i, int max_j, int max_k){
	return {grid.cellMinFaceIdx_u(min_i, min_j, min_k), grid.cellMinFaceIdx_u(max_i, min_j, min_k),
			grid.cellMinFaceIdx_u(min_i, min_j, max_k), grid.cellMinFaceIdx_u(max_i, min_j, max_k),
			grid.cellMinFaceIdx_u(min_i, max_j, min_k), grid.cellMinFaceIdx_u(max_i, max_j, min_k),
			grid.cellMinFaceIdx_u(min_i, max_j, max_k), grid.cellMinFaceIdx_u(max_i, max_j, max_k)};
}

tuple8i getStageredCellFaceNBRIdcs_v(MacGrid const & grid, int min_i, int min_j, int min_k, int max_i, int max_j, int max_k){
	return {grid.cellMinFaceIdx_v(min_i, min_j, min_k), grid.cellMinFaceIdx_v(max_i, min_j, min_k),
			grid.cellMinFaceIdx_v(min_i, min_j, max_k), grid.cellMinFaceIdx_v(max_i, min_j, max_k),
			grid.cellMinFaceIdx_v(min_i, max_j, min_k), grid.cellMinFaceIdx_v(max_i, max_j, min_k),
			grid.cellMinFaceIdx_v(min_i, max_j, max_k), grid.cellMinFaceIdx_v(max_i, max_j, max_k)};
}

tuple8i getStageredCellFaceNBRIdcs_w(MacGrid const & grid, int min_i, int min_j, int min_k, int max_i, int max_j, int max_k){
	return {grid.cellMinFaceIdx_w(min_i, min_j, min_k), grid.cellMinFaceIdx_w(max_i, min_j, min_k),
			grid.cellMinFaceIdx_w(min_i, min_j, max_k), grid.cellMinFaceIdx_w(max_i, min_j, max_k),
			grid.cellMinFaceIdx_w(min_i, max_j, min_k), grid.cellMinFaceIdx_w(max_i, max_j, min_k),
			grid.cellMinFaceIdx_w(min_i, max_j, max_k), grid.cellMinFaceIdx_w(max_i, max_j, max_k)};
}

tuple6i getOrderedNBRIdcs(MacGrid const & grid, Vector3d const & worldSpacePos, int i, int j, int k){
	Vector3d const cellSpacePos = grid.cellSpacePos(worldSpacePos, i, j, k);
	int nbr_i = cellSpacePos[0] < .5f ? i - 1 : i + 1;
	int nbr_j = cellSpacePos[1] < .5f ? j - 1 : j + 1;
	int nbr_k = cellSpacePos[2] < .5f ? k - 1 : k + 1;
	return {std::min(i, nbr_i), std::min(j, nbr_j), std::min(k, nbr_k), 
			std::max(i, nbr_i), std::max(j, nbr_j), std::max(k, nbr_k)};
}

void transferAttributes(MacParticles const & particles, MacGrid & grid){
	auto cellFaceVelSize = grid.cellFaceNum_i * grid.cellFaceNum_j * grid.cellFaceNum_k;

	std::fill(grid.cellFaceVel_u.begin(), grid.cellFaceVel_u.end(), 0);
	std::fill(grid.cellFaceVel_v.begin(), grid.cellFaceVel_v.end(), 0);
	std::fill(grid.cellFaceVel_w.begin(), grid.cellFaceVel_w.end(), 0);

	std::fill(grid.cellFaceWeightSum_u.begin(), grid.cellFaceWeightSum_u.end(), 0);
	std::fill(grid.cellFaceWeightSum_v.begin(), grid.cellFaceWeightSum_v.end(), 0);
	std::fill(grid.cellFaceWeightSum_w.begin(), grid.cellFaceWeightSum_w.end(), 0);

	//pi is particle index
	for(int pi = 0 ; pi < particles.size ; pi++){
		auto & pPos = particles.pos[pi];
		auto & pVel = particles.vel[pi];
		auto [i, j, k] = grid.gridCoord(pPos);
		auto [min_i, min_j, min_k, max_i, max_j, max_k] = getOrderedNBRIdcs(grid, pPos, i, j, k);
		
		tuple8i cellUFaceNBRIdcs = getStageredCellFaceNBRIdcs_u(grid, min_i, min_j, min_k, max_i, max_j, max_k);
		auto cellFacePosc000_u = grid.cellMinFacePos_u(min_i, min_j, min_k);
		auto cellFacePosc111_u = grid.cellMinFacePos_u(max_i, max_j, max_k);
		auto weights_u = getWeights(pPos, cellFacePosc000_u, cellFacePosc111_u);
		for(int ii=0 ; ii<8 ; ii++){
			auto fi_u = cellUFaceNBRIdcs[ii];
			grid.cellFaceVel_u[fi_u] += weights_u[ii] * pVel[0];
			grid.cellFaceWeightSum_u[fi_u] += weights_u[ii];
		}

		tuple8i cellVFaceNBRIdcs = getStageredCellFaceNBRIdcs_v(grid, min_i, min_j, min_k, max_i, max_j, max_k);
		auto cellFacePosc000_v = grid.cellMinFacePos_v(min_i, min_j, min_k);
		auto cellFacePosc111_v = grid.cellMinFacePos_v(max_i, max_j, max_k);
		auto weights_v = getWeights(pPos, cellFacePosc000_v, cellFacePosc111_v);
		for(int ii=0 ; ii<8 ; ii++){
			auto fi_v = cellVFaceNBRIdcs[ii];
			grid.cellFaceVel_v[fi_v] += weights_v[ii] * pVel[1];
			grid.cellFaceWeightSum_v[fi_v] += weights_v[ii];
		}


		tuple8i cellWFaceNBRIdcs = getStageredCellFaceNBRIdcs_w(grid, min_i, min_j, min_k, max_i, max_j, max_k);
		auto cellFacePosc000_w = grid.cellMinFacePos_w(min_i, min_j, min_k);
		auto cellFacePosc111_w = grid.cellMinFacePos_w(max_i, max_j, max_k);
		auto weights_w = getWeights(pPos, cellFacePosc000_w, cellFacePosc111_w);
		for(int ii=0 ; ii<8 ; ii++){
			auto fi_w = cellWFaceNBRIdcs[ii];
			grid.cellFaceVel_w[fi_w] += weights_w[ii] * pVel[2];
			grid.cellFaceWeightSum_w[fi_w] += weights_w[ii];
		}
	}
	for (int i_u = 0; i_u < grid.cellFaceVel_u.size() ; i_u++)
		if(grid.cellFaceWeightSum_u[i_u])
			grid.cellFaceVel_u[i_u] /= grid.cellFaceWeightSum_u[i_u];
	for (int i_v = 0; i_v < grid.cellFaceVel_v.size() ; i_v++)
		if(grid.cellFaceWeightSum_v[i_v])
			grid.cellFaceVel_v[i_v] /= grid.cellFaceWeightSum_v[i_v];
	for (int i_w = 0; i_w < grid.cellFaceVel_w.size() ; i_w++)
		if(grid.cellFaceWeightSum_w[i_w])
			grid.cellFaceVel_w[i_w] /= grid.cellFaceWeightSum_w[i_w];
		
}

void transferAttributes(MacGrid const & grid, MacParticles & particles){
	//pi is particle index
	for(int pi = 0 ; pi < particles.size ; pi++){
		auto & pPos = particles.pos[pi];
		auto & pVel = particles.vel[pi];
		auto [i, j, k] = grid.gridCoord(pPos);
		auto [min_i, min_j, min_k, max_i, max_j, max_k] = getOrderedNBRIdcs(grid, pPos, i, j, k);

		auto cellFacePosc000_u = grid.cellMinFacePos_u(min_i, min_j, min_k);
		auto cellFacePosc111_u = grid.cellMinFacePos_u(max_i, max_j, max_k);
		tuple8i cellUFaceNBRIdcs = getStageredCellFaceNBRIdcs_u(grid, min_i, min_j, min_k, max_i, max_j, max_k);
		auto cellFaceNBRVels_u = getFromIdcs(grid.cellFaceVel_u, cellUFaceNBRIdcs);
		pVel[0] = trilinearInterpolation(cellFaceNBRVels_u, getDiff(pPos, cellFacePosc000_u, cellFacePosc111_u));

		auto cellFacePosc000_v = grid.cellMinFacePos_v(min_i, min_j, min_k);
		auto cellFacePosc111_v = grid.cellMinFacePos_v(max_i, max_j, max_k);
		tuple8i cellVFaceNBRIdcs = getStageredCellFaceNBRIdcs_v(grid, min_i, min_j, min_k, max_i, max_j, max_k);
		auto cellFaceNBRVels_v = getFromIdcs(grid.cellFaceVel_v, cellVFaceNBRIdcs);
		pVel[1] = trilinearInterpolation(cellFaceNBRVels_v, getDiff(pPos, cellFacePosc000_v, cellFacePosc111_v));

		auto cellFacePosc000_w = grid.cellMinFacePos_w(min_i, min_j, min_k);
		auto cellFacePosc111_w = grid.cellMinFacePos_w(max_i, max_j, max_k);
		tuple8i cellWFaceNBRIdcs = getStageredCellFaceNBRIdcs_w(grid, min_i, min_j, min_k, max_i, max_j, max_k);
		auto cellFaceNBRVels_w = getFromIdcs(grid.cellFaceVel_w, cellWFaceNBRIdcs);
		pVel[2] = trilinearInterpolation(cellFaceNBRVels_w, getDiff(pPos, cellFacePosc000_w, cellFacePosc111_w));
	}
}

double calculateSubStep(MacGrid const & grid, double timeStep){

	double maxVelComponent = cellSize;
	for ( size_t i = 0 ; i < grid.cellFaceNum_i; i++)
		for ( size_t j = 0 ; j < grid.cellFaceNum_j; j++)
			for ( size_t k = 0 ; k < grid.cellFaceNum_k; k++){
				if(grid.cellFaceVel_u[i] * timeStep > maxVelComponent)
					maxVelComponent = grid.cellFaceVel_u[i] * timeStep;
				if(grid.cellFaceVel_v[j] * timeStep > maxVelComponent)
					maxVelComponent = grid.cellFaceVel_v[j] * timeStep;
				if(grid.cellFaceVel_w[k] * timeStep > maxVelComponent)
					maxVelComponent = grid.cellFaceVel_w[k] * timeStep;
			}
	double subStep = cellSize / maxVelComponent;
	return subStep;
}

void applyExternalForces(MacGrid const & grid, double subStep){

}

/**

void transferAttributes(MacGrid const & grid, MacParticles & particles){
	//pi is particle index
	for(int pi = 0 ; pi < particles.size ; pi++){
		auto & pPos = particles.pos[pi];
		auto & pVel = particles.vel[pi];
		auto [i, j, k] = grid.gridCoord(pPos);
		auto [min_i, min_j, min_k, max_i, max_j, max_k] = getOrderedNBRIdcs(grid, pPos, i, j, k);

		pVel = {0,0,0};

		double cornerCoeff = 1.f/8.f;

		auto cellFacePosc000_u = grid.cellMinFacePos_u(min_i, min_j, min_k);
		auto cellFacePosc111_u = grid.cellMinFacePos_u(max_i, max_j, max_k);
		tuple8i cellUFaceNBRIdcs = getStageredCellFaceNBRIdcs_u(grid, min_i, min_j, min_k, max_i, max_j, max_k);
		auto weights_u = getWeights(pPos, cellFacePosc000_u, cellFacePosc111_u);
		std::cout<<"weights_u:\n";
		for(int ii=0 ; ii<8 ; ii++){
			pVel[0] += grid.cellFaceVel_u[cellUFaceNBRIdcs[ii]] * cornerCoeff / weights_u[ii];
			std::cout<<weights_u[ii]<<"\n";
		}

		auto cellFacePosc000_v = grid.cellMinFacePos_v(min_i, min_j, min_k);
		auto cellFacePosc111_v = grid.cellMinFacePos_v(max_i, max_j, max_k);
		tuple8i cellVFaceNBRIdcs = getStageredCellFaceNBRIdcs_v(grid, min_i, min_j, min_k, max_i, max_j, max_k);
		auto weights_v = getWeights(pPos, cellFacePosc000_v, cellFacePosc111_v);
		std::cout<<"weights_v:\n";
		for(int ii=0 ; ii<8 ; ii++){
			pVel[1] += grid.cellFaceVel_v[cellVFaceNBRIdcs[ii]] * cornerCoeff / weights_v[ii];
			std::cout<<weights_v[ii]<<"\n";
		}

		auto cellFacePosc000_w = grid.cellMinFacePos_w(min_i, min_j, min_k);
		auto cellFacePosc111_w = grid.cellMinFacePos_w(max_i, max_j, max_k);
		tuple8i cellWFaceNBRIdcs = getStageredCellFaceNBRIdcs_w(grid, min_i, min_j, min_k, max_i, max_j, max_k);
		auto weights_w = getWeights(pPos, cellFacePosc000_w, cellFacePosc111_w);
		std::cout<<"weights_w:\n";
		for(int ii=0 ; ii<8 ; ii++){
			pVel[2] += grid.cellFaceVel_w[cellWFaceNBRIdcs[ii]] * cornerCoeff / weights_w[ii];
			std::cout<<weights_w[ii]<<"\n";
		}
	}
}

**/

}