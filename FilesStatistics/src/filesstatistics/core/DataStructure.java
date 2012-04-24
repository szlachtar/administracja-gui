/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.core;

/**
 *
 * @author Rafal
 */
public class DataStructure {

    public DataStructure() {
    }

    public DataStructure(String path, int read, int write) {
        this.path = path;
        this.read = read;
        this.write = write;
    }

    private String path;

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    private int read;

    public int getRead() {
        return read;
    }

    public void setRead(int read) {
        this.read = read;
    }
    private int write;

    public int getWrite() {
        return write;
    }

    public void setWrite(int write) {
        this.write = write;
    }

}
