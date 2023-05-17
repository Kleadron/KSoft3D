#include "includes_base.h"
#include "r_core.h"
#include "r_3d.h"
#include "globals_base.h"
#include "defines_base.h"
#include "epicfail.h"

typedef struct ObjFaceVertex
{
	int vertex;
	int normal;
	int uv;

} ObjFaceVertex;

typedef struct ObjFace
{
	int firstVertex;
	int numVertices;

} ObjFace;

typedef struct R_MeshInfo
{
	bool allocated;
	char *filePath;

	vec3 *vertices;
	int numVertices;
	vec3 *normals;
	int numNormals;
	vec2 *uvs;
	int numUVs;

	bool hasNormals;
	bool hasUVs;

	ObjFaceVertex *faceverts;
	ObjFace *faces;

	int numFaceVerts;
	int numFaces;

	int displayList;

} R_3D_MeshInfo;

#define MAX_MODELS 16
R_3D_MeshInfo models[MAX_MODELS];
R_3D_MeshID nextModelSlot = 0;

void R_3D_Init()
{
	memset(models, 0, sizeof(models));
}

void SubmitMesh(R_3D_MeshInfo *mesh)
{
	int lastNormal = -1;

	for (int i = 0; i < mesh->numFaces; i++)
	{
		ObjFace *face = &mesh->faces[i];

		glBegin(GL_POLYGON);
		for (int j = 0; j < face->numVertices; j++)
		{
			ObjFaceVertex *facevert = &mesh->faceverts[face->firstVertex + j];

			if (mesh->hasUVs)
			{
				vec2 *uv = &mesh->uvs[facevert->uv];
				glTexCoord2fv(uv);
			}

			if (mesh->hasNormals)
			{
				if (facevert->normal != lastNormal)
				{
					vec3 *normal = &mesh->normals[facevert->normal];
					glNormal3fv(normal);
					lastNormal = facevert->normal;
				}
			}

			vec3 *vertex = &mesh->vertices[facevert->vertex];
			glVertex3fv(vertex);
		}
		glEnd();

		R_CheckError();
		//break;
	}
}

// Loads an OBJ mesh from the specified path
R_3D_MeshID R_3D_LoadMesh(const char *path, const float scale)
{
	CheckFail(path == NULL, 0, "R_3D_LoadMesh path is null", path, FAILFLAG_CANCONTINUE);

	CheckFail(file_exists(path), 1, "R_3D_LoadMesh file doesn't exist", path, 0);

	// check if this was already loaded
	for (int i = 0; i < MAX_MODELS; i++)
	{
		if (models[i].allocated)
		{
			if (strcmp(path, models[i].filePath) == 0)
			{
				// give the existing id
				return i;
			}
		}
	}


	R_3D_MeshID slot = nextModelSlot++;
	R_3D_MeshInfo *model = &models[slot];
	model->allocated = true;
	// keep track of already loaded models
	model->filePath = (char*)malloc(strlen(path) + 1);
	strcpy(model->filePath, path);


	FILE *modelFile = fopen(path, "r");
	char linebuf[255];
	while (true)
	{
		char *linestr = fgets(linebuf, sizeof(linebuf), modelFile);

		if (linestr == NULL)
		{
			break;
		}
		if (strlen(linestr) == 0)
		{
			continue;
		}
		if (linestr[0] == '#')
		{
			continue;
		}

		char *linetoken = strtok(linestr, " ");

		if (strcmp(linetoken, "v") == 0)
		{
			float x = atof(strtok(NULL, " "));
			float y = atof(strtok(NULL, " "));
			float z = atof(strtok(NULL, " "));

			model->numVertices++;
			model->vertices = realloc(model->vertices, sizeof(vec3) * model->numVertices);

			vec3 vert = {x * scale, y * scale, z * scale};
			glm_vec3_copy(vert, model->vertices[model->numVertices - 1]);
		}

		if (strcmp(linetoken, "vn") == 0)
		{
			float x = atof(strtok(NULL, " "));
			float y = atof(strtok(NULL, " "));
			float z = atof(strtok(NULL, " "));

			model->numNormals++;
			model->normals = realloc(model->normals, sizeof(vec3) * model->numNormals);
			model->hasNormals = true;

			vec3 vert = { x, y, z };
			glm_vec3_copy(vert, model->normals[model->numNormals - 1]);
		}

		if (strcmp(linetoken, "vt") == 0)
		{
			float u = atof(strtok(NULL, " "));
			float v = atof(strtok(NULL, " "));

			model->numUVs++;
			model->uvs = realloc(model->uvs, sizeof(vec2) * model->numUVs);
			model->hasUVs = true;

			vec2 uv = { u, v };
			glm_vec2_copy(uv, model->uvs[model->numUVs - 1]);
		}

		if (strcmp(linetoken, "f") == 0)
		{
			int facevertsBefore = model->numFaceVerts;
			int numFaceverts = 0;

			char *vertToken = strtok(NULL, " ");
			while (vertToken != NULL)
			{
				// will be closed off when the other tokens are scanned
				char *vert = vertToken;
				vertToken = strtok(NULL, " ");

				bool readUV = false;
				bool readNorm = false;

				char *uv = strchr(vert, '/');
				if (uv != NULL)
				{
					*uv = NULL;
					uv++;
				}
				char *norm = strchr(uv, '/');
				if (norm != NULL)
				{
					*norm = NULL;
					norm++;
				}

				if (uv != NULL && strlen(uv) > 0)
					readUV = true;
				if (norm != NULL && strlen(norm) > 0)
					readNorm = true;

				int vertindex = atoi(vert);
				int uvindex = readUV ? atoi(uv) : 0;
				int normindex = readNorm ? atoi(norm) : 0;

				model->numFaceVerts++;
				model->faceverts = realloc(model->faceverts, sizeof(ObjFaceVertex) * model->numFaceVerts);
				
				ObjFaceVertex *facevert = &model->faceverts[model->numFaceVerts - 1];

				facevert->vertex = vertindex - 1;
				facevert->uv = uvindex - 1;
				facevert->normal = normindex - 1;
				numFaceverts++;
			}

			model->numFaces++;
			model->faces = realloc(model->faces, sizeof(ObjFace) * model->numFaces);

			ObjFace *face = &model->faces[model->numFaces - 1];
			face->firstVertex = facevertsBefore;
			face->numVertices = numFaceverts;
		}
	}

	fclose(modelFile);

	model->displayList = glGenLists(1);

	glNewList(model->displayList, GL_COMPILE);
	SubmitMesh(model);
	glEndList();

	return slot;
}



void R_3D_DrawMesh(float x, float y, float z, float pitch, float yaw, R_ColorRGBA color, R_3D_MeshID modelID, R_TexID texID)
{
	R_3D_MeshInfo *model = &models[modelID];

	if (model->hasUVs)
	{
		glEnable(GL_TEXTURE_2D);
		R_BindTex(texID);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}

	//glPolygonMode(GL_FRONT, GL_LINE);
	//glPolygonMode(GL_BACK, GL_LINE);

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(yaw, 0, 1, 0);
	glRotatef(pitch, 1, 0, 0);

	glColor4ubv((unsigned char*)&color);
	//glBegin(GL_TRIANGLES);

	glCallList(model->displayList);

	//glEnd();

	glPopMatrix();

	//glPolygonMode(GL_FRONT, GL_FILL);
	//glPolygonMode(GL_BACK, GL_FILL);
}

void R_3D_ApplyProjection(const float fov, const float aspectRatio, const float nearClip, const float farClip)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(fov, aspectRatio, nearClip, farClip);

	glMatrixMode(GL_MODELVIEW);
}

void R_3D_SetViewpoint(float x, float y, float z, float pitch, float yaw)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glTranslatef(0.0f, 0.0f, -0.3f);
	glRotatef(-pitch, 1.0f, 0.0f, 0.0f);
	glRotatef(-yaw, 0.0f, 1.0f, 0.0f);
	glTranslatef(-x, -y, -z);
}