pipeline {

    agent any
    stages{
          
        stage('Build') {
            steps{
                sh 'echo cmake...'
                sh 'mkdir build'
                sh 'cd build'
                sh 'cmake ..'
                sh 'make'
                
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
