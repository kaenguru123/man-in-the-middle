pipeline {

    agent any
    stages{
          
        stage('Build') {
            steps{
                sh 'echo cmake...'
                sh 'pwd'
                sh 'sudo rm -rf build'
                sh 'sudo mkdir build'
                sh 'sudo cd build'
                sh 'pwd'
                sh 'ls'
                sh 'ls ..'
                sh 'sudo cmake ..'
                sh 'sudo make'
                
            }
        }
        
        stage('Test') { 
            steps{            
                sh 'echo MyTest'
            }
        }
        
        stage('Deploy') { 
            steps{            
                sh 'echo MyDeploy'
            }
        }
    }
}
