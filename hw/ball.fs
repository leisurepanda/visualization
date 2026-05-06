#version 330 core
out vec4 FragColor;
uniform int conum;

in vec2 TexCoord;
uniform sampler2D texture1;

/*void main(){
	if(conum==0){
	FragColor = vec4(0.5,0.0,0.0,1.0);
	}else if(conum==1){
		FragColor = vec4(0.0,0.5,0.0,1.0);
	}else if(conum==2){
		FragColor = vec4(0.0,0.0,0.5,1.0);
	}
}*/

void main(){
	FragColor = texture(texture1,TexCoord);
	//FragColor = texture(texture1,TexCoord);
}